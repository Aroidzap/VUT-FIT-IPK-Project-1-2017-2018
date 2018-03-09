/*
*	IPK Project 1: client-server for simple file transfer
*	Author: Tomáš Pazdiora (xpazdi02)
*	File: IPKFTP.cpp
*/

#include "IPKFTP.h"

#include "IPKPacket.h"

#include <iostream>
#include <fstream>
#include <iterator>
#include <stdexcept>
#include <thread>

const int IPKFTP::retries = 2; // total number of tries = 1 + retries

// -------------- File Methods --------------

std::vector<unsigned char> IPKFTP::FileLoad(std::string filename)
{
	std::vector<unsigned char> data;
	std::ifstream file;
	file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	file.open(filename, std::ios::binary);
	std::copy(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>(), std::back_inserter(data));
	return data;
}

void IPKFTP::FileSave(std::string filename, std::vector<unsigned char> data)
{
	std::ofstream file;
	file.exceptions(std::ofstream::failbit | std::ofstream::badbit);
	file.open(filename, std::ios::binary);
	std::copy(std::begin(data), std::end(data), std::ostreambuf_iterator<char>(file));
}

// ------------------------------------------

bool IPKFTP::ServerStart(std::string port)
{
	//Possible Improvement: std::cout logging
	//Possible Improvement: split large files
	//Possible Improvement: enable termination of server using stdin

	tcp.Listen(port, [](TCP client) {
		std::thread thread(ServerThreadCode, std::ref(client));
		thread.detach(); //detach thread to be ready to accept another client without blocking
	}); // infinite loop 

	return true;
}

void IPKFTP::ServerThreadCode(TCP &client) {
	for (int i = 0; i <= retries; i++) {
		try {
			bool close = false;
			while (!close) { // loop until client closes connection, or until (1 + retries) * timeout
				std::vector<unsigned char> packet{};
				switch (IPKPacket::Type(packet = client.Recv(IPKPacket::StatusSize))) {
				case CommandPing:
				{
					client.Send(IPKPacket(StatusOk));
					break;
				}
				case OfferFile:
				{
					client.Recv(packet, IPKPacket::ExpectedSize(packet) - IPKPacket::StatusSize);
					IPKPacket p(packet);
					FileSave(p.GetFilename(), p.GetData());
					client.Send(IPKPacket(StatusOk));
					break;
				}
				case RequestFile:
				{
					client.Recv(packet, IPKPacket::ExpectedSize(packet) - IPKPacket::StatusSize);
					auto filename = IPKPacket(packet).GetFilename();
					auto data = FileLoad(filename);
					client.Send(IPKPacket(OfferFile, filename, data));
					break;
				}
				default:
					client.Send(IPKPacket(StatusError));
					break;
				}
			}
		}
		catch (const TCPException &e) {
			if (e.error == Timeout) {
				client.Send(IPKPacket(StatusError));
			}
			else if (e.error == ConnectionClosed) {
				break; //close connection
			}
			else {
				throw;
			}
		}
		catch (const IPKPacketException &e) {
			if (e.error == SignatureError || e.error == VersionError || e.error == TransmissionTypeError ||
				e.error == SizeError || e.error == CRC32Error) {
				client.Send(IPKPacket(StatusError));
			}
			else {
				throw;
			}
		}
		catch (const std::fstream::failure &e) {
			(void)e; // bypass unreferenced local variable warning
			client.Send(IPKPacket(StatusInaccessible));
			break; //close connection
		}
	}
	client.Close();
}

void IPKFTP::ServerStop()
{
	//not needed for now, since server starts an infinite loop
}

bool IPKFTP::ClientConnect(std::string host, std::string port)
{
	//Possible Improvement: std::cout logging
	if (tcp.IsConnected()) {
		tcp.Close();
	}
	for (int i = 0; i <= retries; i++) {
		try {
			tcp.Connect(host, port);
			tcp.Send(IPKPacket(CommandPing));
			if (IPKPacket(tcp.Recv(IPKPacket::StatusSize)) == StatusOk) {
				return true;
			}
			else {
				tcp.Close();
				continue;
			}
		}
		catch (const TCPException &e) {
			if (e.error == ConnectionClosed || e.error == Timeout || e.error == ConnectFailed) {
				tcp.Close();
			}
			else {
				throw;
			}
		}
		catch (const IPKPacketException &e) {
			if (e.error == SignatureError || e.error == VersionError || e.error == TransmissionTypeError || 
				e.error == SizeError || e.error == CRC32Error) {
				tcp.Close();
			} else {
				throw;
			}
		}
	}
	return false;
}

bool IPKFTP::Upload(std::string filename)
{
	//Possible Improvement: std::cout logging
	//Possible Improvement: split large files

	auto filedata = FileLoad(filename);
	
	for (int i = 0; i <= retries; i++) {
		try {
			tcp.Send(IPKPacket(OfferFile, filename, filedata));
			if (IPKPacket(tcp.Recv(IPKPacket::StatusSize)).Type() == StatusOk) {
				return true;
			} 
			else {
				continue;
			}
		}
		catch (const TCPException &e) {
			if (e.error == Timeout) {
				continue;
			}
			else {
				throw;
			}
		}
		catch (const IPKPacketException &e) {
			if (e.error == SignatureError || e.error == VersionError || e.error == TransmissionTypeError ||
				e.error == SizeError || e.error == CRC32Error) {
				continue;
			}
			else {
				throw;
			}
		}
	}
	return false;
}

bool IPKFTP::Download(std::string filename)
{
	//Possible Improvement: std::cout logging
	//Possible Improvement: split large files

	for (int i = 0; i <= retries; i++) {
		try {
			tcp.Send(IPKPacket(RequestFile, filename));
			auto packet = tcp.Recv(IPKPacket::StatusSize);
			tcp.Recv(packet, IPKPacket::ExpectedSize(packet) - IPKPacket::StatusSize);
			IPKPacket p(packet);
			if (p.Type() != OfferFile || p.GetFilename() != filename) { 
				continue; 
			}
			FileSave(p.GetFilename(), p.GetData());
			return true;
		}
		catch (const TCPException &e) {
			if (e.error == Timeout) {
				continue;
			}
			else {
				throw;
			}
		}
		catch (const IPKPacketException &e) {
			if (e.error == SignatureError || e.error == VersionError || e.error == TransmissionTypeError ||
				e.error == SizeError || e.error == CRC32Error) {
				continue;
			}
			else {
				throw;
			}
		}
	}
	return false;
}

void IPKFTP::ClientDisconnect()
{
	tcp.Close();
}