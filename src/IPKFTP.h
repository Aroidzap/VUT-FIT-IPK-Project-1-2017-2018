/*
*	IPK Project 1: client-server for simple file transfer
*	Author: Tomáš Pazdiora (xpazdi02)
*	File: IPKFTP.h
*/

#ifndef IPKFTP_H
#define IPKFTP_H

#include <string>
#include <vector>
#include "TCP.h"

class IPKFTP {
	static const int retries;
	TCP tcp;

	static std::vector<unsigned char> FileLoad(std::string filename);
	static void FileSave(std::string filename, std::vector<unsigned char> data);

	static void ServerThreadCode(TCP &&client);
public:
	void ServerStart(std::string port);
	void ServerStop();

	void ClientConnect(std::string host, std::string port);
	void ClientDisconnect();

	void Upload(std::string filename);
	void Download(std::string filename);
};

#endif
