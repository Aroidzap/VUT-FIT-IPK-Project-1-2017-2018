/*
*	IPK Project 1: client-server for simple file transfer
*	Author: Tomáš Pazdiora (xpazdi02)
*	File: client.cpp
*/

#include <iostream>
#include <string>
#include "IPKFTP.h"

const std::string client_usage = "./ipk-client -h host -p port [-r|-w] file";

struct args {
	std::string host, port, filename;
	char mode;
} arguments;

bool load_args(int argc, const char *argv[], args *arguments);

/*int main(int argc, const char *argv[]) {
	if (!load_args(argc, argv, &arguments)) {
		std::cerr << client_usage << std::endl;
		return -1;
	}

	// TODO: handle errors
	IPKFTP ipkftp;
	ipkftp.ClientConnect(arguments.host, arguments.port);
	if (arguments.mode == 'w') {
		ipkftp.Upload(arguments.filename);
	}
	else {
		ipkftp.Download(arguments.filename);
	}
	ipkftp.ClientDisconnect();

	return 0;
};*/
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <netdb.h>

#include "TCP.h"
int main() {
	TCP tcp;
	std::vector<unsigned char> buffer;
	tcp.Connect("www.seznam.cz", "80");
	std::string req = "GET /\r\n";
	tcp.Send(std::vector<unsigned char>(req.begin(), req.end()));
	tcp.Recv(buffer, 100000);
	std::string response(buffer.begin(), buffer.end());
	std::cout << response;
	tcp.Close();
}

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