/*
*	IPK Project 1: client-server for simple file transfer
*	Author: Tomáš Pazdiora (xpazdi02)
*	File: IPKFTP.h
*/

#ifndef IPKFTP_H
#define IPKFTP_H

#include <string>
#include <vector>

class IPKFTP {
	static const int retries;
	bool connected;
	bool FileExists(std::string filename);
	std::vector<unsigned char> FileLoad(std::string filename);
	void FileSave(std::string filename, std::vector<unsigned char> data);
public:
	bool ServerModeEnable(std::string port);
	void ServerModeDisable();

	bool ClientConnect(std::string host, std::string port);
	void ClientDisconnect();

	bool Upload(std::string filename);
	bool Download(std::string filename);

	bool Upload(std::string host, std::string port, std::string filename);
	bool Download(std::string host, std::string port, std::string filename);
};

#endif
