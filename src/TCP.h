/*
*	IPK Project 1: client-server for simple file transfer
*	Author: Tomáš Pazdiora (xpazdi02)
*	File: TCP.h
*/

#ifndef TCP_H
#define TCP_H

#include <string>
#include <stdexcept>
#include <functional>
#include <vector>

// Linux specific
#ifdef __linux__
using TCPSocket = int;
#endif

// Windows specific
#ifdef _WIN32
#include <winsock2.h> //requires ws2_32.lib
using TCPSocket = SOCKET;
#endif

enum TCPError {
	ConnectionClosed,
	Timeout,
	AddrPortInUse,
	ConnectFailed,
	ListenFailed,
	BindingFailed,
	SelectFailed,
	SendRecvFailed,
	setNonBlockingFailed,
	PlatformSpecificError
};

class TCP {
public:
	static const int maxconnections;
	static const bool nonblocking;
	const std::size_t block_size;
	const int timeout;

	bool connected;
	TCPSocket sock;

	bool moved;
	bool setNonBlocking(TCPSocket socket);

public:
	TCP();
	TCP(TCPSocket socket);
	TCP(const TCP & other) = delete;
	TCP(TCP && other);
	~TCP();

	void Connect(std::string host, std::string port);

	// Listen => host == specific interface
	void Listen(std::string port, std::function<void(TCP)> clientConnectionHandler, std::string host = {}); 
	void Listen(std::string port, std::function<void(TCP, const std::string, const std::string)> clientConnectionHandler, std::string host = {});

	void Close();

	// blocking recv with timeout and periodical update callback
	void Recv(std::vector<unsigned char> &data, std::size_t bytes, std::function<void(std::size_t)> update = {});
	// blocking send with timeout and periodical update callback
	void Send(const std::vector<unsigned char> &data, std::function<void(std::size_t)> update = {});
};

class TCPException : public std::runtime_error {
public:
	const TCPError error;
	TCPException(const TCPError error, const std::string message = "TCPError");
};

#endif