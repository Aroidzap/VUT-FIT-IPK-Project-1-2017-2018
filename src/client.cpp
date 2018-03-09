/*
*	IPK Project 1: client-server for simple file transfer
*	Author: Tomáš Pazdiora (xpazdi02)
*	File: client.cpp
*/

#include <iostream>
#include <string>
#include <fstream>
#include "IPKFTP.h"

const std::string client_usage = "./ipk-client -h host -p port [-r|-w] file";

struct args {
	std::string host, port, filename;
	char mode;
} arguments;

bool load_args(int argc, const char *argv[], args *arguments);

int main(int argc, const char *argv[]) {
	if (!load_args(argc, argv, &arguments)) {
		std::cerr << client_usage << std::endl;
		return -1;
	}

	try {
		IPKFTP ipkftp;
		ipkftp.ClientConnect(arguments.host, arguments.port);
		if (arguments.mode == 'w') {
			ipkftp.Upload(arguments.filename);
		}
		else {
			ipkftp.Download(arguments.filename);
		}
		ipkftp.ClientDisconnect();
	}
	catch (const std::ifstream::failure &e) {
		(void)e; // bypass unreferenced local variable warning
		if (arguments.mode == 'w') {
			std::cerr << "Error: Unable to open file!" << std::endl;
		}
		else {
			std::cerr << "Error: Unable to save file!" << std::endl;
		}
	}
	catch (const std::exception &e) {
		std::cerr << e.what() << std::endl;
	}

	return 0;
};

bool load_args(int argc, const char *argv[], args *arguments) {
	bool host(false), port(false), mode(false);
	if (argc == 7) {
		for (int i = 1; i < 6; i += 2) {
			if (std::string(argv[i]) == "-h" && !host) {
				arguments->host = std::string(argv[i + 1]); host = true;
			}
			else if (std::string(argv[i]) == "-p" && !port) {
				arguments->port = std::string(argv[i + 1]); port = true;
			}
			else if (std::string(argv[i]) == "-r" && !mode) {
				arguments->filename = std::string(argv[i + 1]);
				arguments->mode = 'r'; mode = true;
			}
			else if (std::string(argv[i]) == "-w" && !mode) {
				arguments->filename = std::string(argv[i + 1]);
				arguments->mode = 'w'; mode = true;
			}
			else {
				return false;
			}
		}
		return true;
	}
	else {
		return false;
	}
}