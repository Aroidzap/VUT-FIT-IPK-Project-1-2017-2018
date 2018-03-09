/*
*	IPK Project 1: client-server for simple file transfer
*	Author: Tomáš Pazdiora (xpazdi02)
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

int main(int argc, const char *argv[]) 
{
	if (!load_args(argc, argv, &arguments)) {
		std::cerr << server_usage << std::endl;
		return -1;
	}

	try {
		IPKFTP ipkftp;
		ipkftp.ServerStart(arguments.port); // infinite loop for now
		ipkftp.ServerStop(); // reserved for future
	}
	catch (const std::exception &e){
		std::cerr << e.what() << std::endl;
	}

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