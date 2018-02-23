/*
*	IPK Project 1: client-server for simple file transfer
*	Author: Tomáš Pazdiora (xpazdi02)
*	File: TCP.cpp
*/

#include "TCP.h"

//#include <arpa/inet.h>
//#include <netinet/in.h>

#include <string>
#include <algorithm>

#include <cstring>

#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <netdb.h>

TCP::TCP() : timeout(7), block_size(1024), connected(false), sock(-1)
{
}

bool TCP::Connect(std::string host, std::string port)
{
	struct addrinfo hints, *result;

	std::memset(&hints, 0, sizeof(addrinfo));
	hints.ai_family = AF_UNSPEC; // IPv4 or IPv4
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP; // TCP

	if (getaddrinfo(host.c_str(), port.c_str(), &hints, &result) != 0) {
		return this->connected;
	}

	for (auto res = result; res != NULL; res = res->ai_next) {
		this->sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
		if (this->sock != -1) {
			if (connect(this->sock, res->ai_addr, res->ai_addrlen) != -1) {
				freeaddrinfo(result);
				this->connected = true;
				break;
			}
			else {
				close(this->sock);
			}
		}
	}
	return this->connected;
}

bool TCP::Listen(std::string port)
{
	// TODO
	throw(std::runtime_error("not implemented!"));
	return false;
}

void TCP::Close()
{
	shutdown(sock, SHUT_RDWR);
	close(sock);
}

void TCP::Recv(std::vector<unsigned char>& data, std::size_t bytes, std::function<void(std::size_t)> update)
{
	//TODO: set nonblocking fcntl
	fd_set rfds;
	timeval time_out;

	time_out.tv_sec = this->timeout;
	time_out.tv_usec = 0;

	FD_ZERO(&rfds);
	FD_SET(this->sock, &rfds);

	data.resize(data.size() + bytes);

	unsigned char *ptr = &*(data.end() - bytes);
	std::size_t to_read = bytes;

	while (to_read) {
		auto select_ret = select(this->sock + 1, &rfds, NULL, NULL, &time_out);

		if (select_ret == -1) {
			throw TCPException(SelectFailed);
		}
		else if (select_ret) {
			if (FD_ISSET(this->sock, &rfds)) {
				to_read = std::min(this->block_size, to_read);
				std::size_t read = recv(this->sock, ptr, to_read, 0); //TODO: signed/unsigned
				to_read -= read;
				ptr += read;
				if (update) {
					update(bytes - to_read);
				}
			}
		}
		else {
			throw TCPException(Timeout);
		}
	}
}

void TCP::Send(const std::vector<unsigned char>& data, std::function<void(std::size_t)> update)
{
	//TODO: set nonblocking fcntl
	fd_set sfds;
	timeval time_out;

	time_out.tv_sec = this->timeout;
	time_out.tv_usec = 0;

	FD_ZERO(&sfds);
	FD_SET(this->sock, &sfds);

	const unsigned char *ptr = &(*data.begin());
	std::size_t to_write = data.size();

	while (to_write) {
		auto select_ret = select(this->sock + 1, NULL, &sfds, NULL, &time_out);

		if (select_ret == -1) {
			throw TCPException(SelectFailed);
		}
		else if (select_ret) {
			if (FD_ISSET(this->sock, &sfds)) {
				to_write = std::min(this->block_size, to_write);
				std::size_t write = send(this->sock, ptr, to_write, 0); //TODO: signed/unsigned
				to_write -= write;
				ptr += write;
				if (update) {
					update(data.size() - to_write);
				}
			}
		}
		else {
			throw TCPException(Timeout);
		}
	}
}

TCPException::TCPException(const TCPError error, const std::string message)
	: std::runtime_error(message), error(error)
{
}
