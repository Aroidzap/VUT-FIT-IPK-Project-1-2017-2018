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

enum TCPError {
	Timeout,
	SelectFailed
};

class TCP {
public:
	const int timeout;
	const std::size_t block_size;
	bool connected;
	int sock;
public:
	TCP();
	bool Connect(std::string host, std::string port);
	bool Listen(std::string port);
	void Close();

	// blocking recv with timeout and periodical update callback
	void Recv(std::vector<unsigned char> &data, std::size_t bytes, std::function<void(std::size_t)> update = {});
	// blocking send with timeout and periodical update callback
	void Send(const std::vector<unsigned char> &data, std::function<void(std::size_t)> update = {});

};

class TCPException : public std::runtime_error {
	TCPError error;
public:
	TCPException(const TCPError error, const std::string message = "TCPError");
};


#endif