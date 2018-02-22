/*
*	IPK Project 1: client-server for simple file transfer
*	Author: Tomáš Pazdiora (xpazdi02)
*	File: client.cpp
*/

#include <iostream>
#include <string>

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