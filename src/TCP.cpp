/*
*	IPK Project 1: client-server for simple file transfer
*	Author: Tomáš Pazdiora (xpazdi02)
*	File: TCP.cpp
*/

#include "TCP.h"

#include <string>
#include <algorithm>

// Linux specific
#ifdef __linux__
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#endif

// Windows specific
#ifdef _WIN32
#include <ws2tcpip.h>
#undef min

#define select(nfds, r, w, x, t) select(0, (r), (w), (x), (t))

#define connect(s, name, namelen) connect((s), (name), static_cast<int>(namelen))
#define bind(s, name, namelen) bind((s), (name), static_cast<int>(namelen))
#define recv(s, buf, len, flags) recv((s), (buf), static_cast<int>(len), (flags))
#define send(s, buf, len, flags) send((s), (buf), static_cast<int>(len), (flags))

#define close(socket) closesocket(socket)
#define SHUT_RD SD_RECEIVE
#define SHUT_WR SD_SEND
#define SHUT_RDWR SD_BOTH
#endif

// Settings
const int TCP::maxconnections = SOMAXCONN;
const bool TCP::nonblocking = true;
static const std::size_t default_block_size = 1024;
static const int default_timeout = 70;


void TCP::Connect(std::string host, std::string port)
{
	if (this->connected == true) {
		throw(TCPException(ConnectFailed, "TCPError: Connect: Already connected!"));
	}

	struct addrinfo hints {}, *result;

	hints.ai_family = AF_UNSPEC; // IPv4 or IPv4
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP; // TCP

	// convert string address to internal representation
	if (getaddrinfo(host.c_str(), port.c_str(), &hints, &result) != 0) {
		throw(TCPException(ConnectFailed, "TCPError: Unable to resolve host name!")); // getaddrinfo failed
	}

	// connect to one of the results of getaddrinfo
	for (auto res = result; res != NULL; res = res->ai_next) {
		this->sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol); // try to create socket
		if (this->sock != INVALID_SOCKET) {
			if (connect(this->sock, res->ai_addr, res->ai_addrlen) != SOCKET_ERROR) { // try to connect
				freeaddrinfo(result);
				if (nonblocking) { // set non-blocking if enabled
					if(!setNonBlocking(this->sock)){
						shutdown(this->sock, SHUT_RDWR);
						close(this->sock);
						throw(TCPException(setNonBlockingFailed, "TCPError: Unable to make socket non-blocking!"));
					}
				}
				this->connected = true;
				break;
			}
			else {
				close(this->sock);
			}
		}
	}
	if (this->connected == false) {
		throw(TCPException(ConnectFailed, "TCPError: Connect Failed!"));
	}
}

void TCP::Listen(std::string port, std::function<void(TCP)> clientConnectionHandler, std::string host) {
	Listen(port, [clientConnectionHandler](TCP conn, auto client_ip, auto client_port) {clientConnectionHandler(std::move(conn)); }, host);
}
void TCP::Listen(std::string port, std::function<void(TCP, const std::string, const std::string)> clientConnectionHandler, std::string host)
{
	if (this->connected == true) {
		throw(TCPException(ListenFailed, "TCPError: Listen: Already connected!"));
	}

	struct addrinfo hints {0}, *result;

	hints.ai_family = AF_UNSPEC; // IPv4 or IPv4
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP; // TCP
	hints.ai_flags = AI_PASSIVE; // socket will be binded

	const char *hostname = NULL;
	if (!host.empty()) {
		hostname = host.c_str();
	}

	// convert string address to internal representation
	if (getaddrinfo(hostname, port.c_str(), &hints, &result) != 0) {
		throw(TCPException(ListenFailed, "TCPError: Unable to resolve host name!")); // getaddrinfo failed
	}

	// bind and listen to one of the results of getaddrinfo
	for (auto res = result; res != NULL; res = res->ai_next) {
		this->sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol); // try to create socket
		if (this->sock != INVALID_SOCKET) {
			if (bind(this->sock, res->ai_addr, res->ai_addrlen) != SOCKET_ERROR) { // try to bind
				freeaddrinfo(result);
				if (listen(this->sock, maxconnections) == SOCKET_ERROR) { // listen
					close(this->sock);
					throw(TCPException(ListenFailed, "TCPError: listen Failed!"));
				}
				this->connected = true;
				break;
			}
			else {
#ifdef __linux__
				bool addr_port_in_use = (errno == EADDRINUSE);
#endif
#ifdef _WIN32
				bool addr_port_in_use = (WSAGetLastError() == WSAEADDRINUSE);
#endif
				close(this->sock);
				if (addr_port_in_use) {
					throw(TCPException(AddrPortInUse, "TCPError: Address and port are already in use!"));
				}
				else {
					throw(TCPException(BindingFailed, "TCPError: bind Failed!"));
				}
			}
		}
	}
	if (this->connected == false) {
		throw(TCPException(ListenFailed, "TCPError: ListenFailed!"));
	}

	// accept loop (infinite for now) // TODO: enable termination
	bool done = false;
	while (!done) {
		TCPSocket client = accept(this->sock, NULL, NULL); // accept IPv4 and IPv6
		if (client == INVALID_SOCKET) {
			throw(TCPException(ListenFailed, "TCPError: accept Failed!"));
		}

		sockaddr_storage addr_stor; socklen_t addr_len = sizeof(addr_stor);
		sockaddr_in *addr4 = reinterpret_cast<sockaddr_in*>(&addr_stor);
		sockaddr_in6 *addr6 = reinterpret_cast<sockaddr_in6*>(&addr_stor);
		getpeername(client, reinterpret_cast<sockaddr*>(&addr_stor), &addr_len);

		if (nonblocking) { // set non-blocking if enabled
			if (!setNonBlocking(client)) {
				shutdown(client, SHUT_RDWR);
				close(client);
				throw(TCPException(setNonBlockingFailed, "TCPError: Unable to make socket non-blocking!"));
			}
		}
		// call connection handler
		if (clientConnectionHandler) {
			char client_ip_buffer[INET6_ADDRSTRLEN];
			std::string client_port;
			if (addr_stor.ss_family == AF_INET) {
				inet_ntop(addr4->sin_family, &(addr4->sin_addr), client_ip_buffer, sizeof(client_ip_buffer));
				client_port = std::to_string(ntohs(addr4->sin_port));
			} else if (addr_stor.ss_family == AF_INET6) {
				inet_ntop(addr6->sin6_family, &(addr6->sin6_addr), client_ip_buffer, sizeof(client_ip_buffer));
				client_port = std::to_string(ntohs(addr4->sin_port));
			}
			clientConnectionHandler(TCP(client), std::string(client_ip_buffer), client_port);
		}
	}
}

void TCP::Close()
{
	shutdown(this->sock, SHUT_RDWR);
	close(this->sock);
	this->connected = false;
}

bool TCP::IsConnected()
{
	return this->connected;
}

std::vector<unsigned char> TCP::Recv(std::size_t bytes, std::function<void(std::size_t)> update)
{
	std::vector<unsigned char> data;
	Recv(data, bytes, update);
	return data;
}
void TCP::Recv(std::vector<unsigned char>& data, std::size_t bytes, std::function<void(std::size_t)> updateCallback)
{
	//Improvement: use epoll
	fd_set rfds;
	timeval time_out;

	time_out.tv_sec = this->timeout;
	time_out.tv_usec = 0;

	FD_ZERO(&rfds);
	FD_SET(this->sock, &rfds);

	std::size_t to_read = bytes;

	while (to_read) {
		// wait for socket to be ready
		int select_ret = 1;
		if (nonblocking) {
			select_ret = select(this->sock + 1, &rfds, NULL, NULL, &time_out);
		}

		if (select_ret == SOCKET_ERROR) {
			throw TCPException(SelectFailed, "TCPError: SelectFailed!");
		}
		else if (select_ret) {
			if (!(nonblocking) || FD_ISSET(this->sock, &rfds)) {
				std::size_t to_read_current = std::min(this->block_size, to_read); // read up to maximal block size
				data.resize(data.size() + to_read_current);

				char *ptr = reinterpret_cast<char *>(&(*(data.end() - to_read_current)));
				long long recv_ret = recv(this->sock, ptr, to_read_current, 0);
				if (recv_ret == SOCKET_ERROR) {
					throw TCPException(SendRecvFailed, "TCPError: recv Failed!");
				} else if (recv_ret == 0) {
					data.resize(data.size() - to_read_current);
					throw TCPException(ConnectionClosed, "TCPError: Connection Closed!");
				}

				std::size_t read = static_cast<std::size_t>(recv_ret);
				data.resize(data.size() - to_read_current + read);
				to_read -= read;

				if (updateCallback) {
					updateCallback(bytes - to_read); //call optional update callback
				}
			}
		}
		else {
			throw TCPException(Timeout, "TCPError: Timeout!");
		}
	}
}

void TCP::Send(const std::vector<unsigned char>& data, std::function<void(std::size_t)> updateCallback)
{
	//Improvement: use epoll
	fd_set sfds;
	timeval time_out;

	time_out.tv_sec = this->timeout;
	time_out.tv_usec = 0;

	FD_ZERO(&sfds);
	FD_SET(this->sock, &sfds);

	auto it = data.begin();
	std::size_t to_write = data.size();

	while (to_write) {
		// wait for socket to be ready
		int select_ret = 1;
		if (nonblocking) {
			select_ret = select(this->sock + 1, NULL, &sfds, NULL, &time_out);
		}

		if (select_ret == SOCKET_ERROR) {
			throw TCPException(SelectFailed, "TCPError: SelectFailed!");
		}
		else if (select_ret) {
			if (!(nonblocking) || FD_ISSET(this->sock, &sfds)) {
				std::size_t to_write_current = std::min(this->block_size, to_write); // write up to maximal block size
				
				const char *ptr = reinterpret_cast<const char*>(&(*it));
				long long send_ret = send(this->sock, ptr, to_write_current, 0);
				if (send_ret == SOCKET_ERROR) {
					throw TCPException(SendRecvFailed, "TCPError: send Failed!");
				} else if (send_ret == 0) {
					throw TCPException(ConnectionClosed, "TCPError: Connection Closed!");
				}
				
				std::size_t write = static_cast<std::size_t>(send_ret);
				it += write;
				to_write -= write;
				
				if (updateCallback) {
					updateCallback(data.size() - to_write); //call optional update callback
				}
			}
		}
		else {
			throw TCPException(Timeout, "TCPError: Timeout!");
		}
	}
}

bool TCP::setNonBlocking(TCPSocket socket)
{
#ifdef __linux__
	int flags;
	if ((flags = fcntl(socket, F_GETFL, 0)) < 0) {
		return false;
	}
	if (fcntl(socket, F_SETFL, flags | O_NONBLOCK) < 0) {
		return false;
	}
	return true;
#endif
#ifdef _WIN32
	unsigned long mode{1UL};
	return (ioctlsocket(socket, FIONBIO, &mode) == 0);
#endif
}


TCP::TCP() : block_size(default_block_size), timeout(default_timeout), connected(false), sock(INVALID_SOCKET), moved(false) {
#ifdef _WIN32
	// initialize winsock2
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		throw(TCPException(PlatformSpecificError, "TCPError: PlatformSpecificError!"));
	}
#endif
}
TCP::TCP(TCP && other) : block_size(other.block_size), timeout(other.timeout), connected(other.connected), sock(other.sock), moved(other.moved) {
	other.connected = false;
	other.sock = INVALID_SOCKET;
	other.moved = true;
}

TCP::TCP(TCPSocket socket) : TCP() {
	// bypass const for initialization within this constructor
	const_cast<bool &>(connected) = true;
	const_cast<TCPSocket &>(sock) = socket;
}

TCP::~TCP()
{
	if (this->connected) {
		Close();
	}
#ifdef _WIN32
	if (!this->moved) {
		WSACleanup(); // winsock2 cleanup
	}
#endif
}

TCPException::TCPException(const TCPError error, const std::string message)
	: std::runtime_error(message), error(error)
{
}