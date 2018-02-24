/*
*	IPK Project 1: client-server for simple file transfer
*	Author: Tom� Pazdiora (xpazdi02)
*	File: server.cpp
*/

#include <iostream>
#include <string>
#include "IPKFTP.h"

const std::string server_usage = "./ipk-server -p port ";

struct args {
	std::string port;
} arguments;

bool load_args(int argc, const char *argv[], args *arguments);

int main(int argc, const char *argv[]) {
	if (!load_args(argc, argv, &arguments)) {
		std::cerr << server_usage << std::endl;
		return -1;
	}

	// TODO: handle errors
	IPKFTP ipkftp;
	ipkftp.ServerModeEnable(arguments.port); // infinite loop for now
	ipkftp.ServerModeDisable(); // reserved for future

	return 0;
};

bool load_args(int argc, const char *argv[], args *arguments) {
	if (argc == 3 && std::string(argv[1]) == "-p") {
		arguments->port = argv[2];
		return true;
	}
	else {
		return false;
	}
}