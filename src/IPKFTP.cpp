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

const int IPKFTP::retries = 3;

bool IPKFTP::FileExists(std::string filename)
{
	return std::ifstream(filename).good();
}

std::vector<unsigned char> IPKFTP::FileLoad(std::string filename)
{
	std::vector<unsigned char> data;
	std::ifstream file(filename, std::ios::binary);

	std::copy(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>(), std::back_inserter(data));

	return data;
}

void IPKFTP::FileSave(std::string filename, std::vector<unsigned char> data)
{
	std::ofstream file(filename, std::ios::binary);

	std::copy(std::begin(data), std::end(data), std::ostreambuf_iterator<char>(file));
}

bool IPKFTP::ServerModeEnable(std::string port)
{
	//Possible Improvement: std::cout logging
	//Possible Improvement: split large files

	tcp.Listen(port, [](TCP client) // TODO: enable termination, use threads
	{
		bool close = false;
		while (!close) {
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
				close = true;
				break;
			}
			case RequestFile:
			{
				client.Recv(packet, IPKPacket::ExpectedSize(packet) - IPKPacket::StatusSize);
				auto filename = IPKPacket(packet).GetFilename();
				auto data = FileLoad(filename);
				client.Send(IPKPacket(OfferFile, filename, FileLoad(filename)));
				if (!(IPKPacket(client.Recv(IPKPacket::StatusSize)) == StatusOk)) {
					//TODO: retry
				}
				close = true;
				break;
			}
			default:
				break;
			}
		}
	}); // infinite loop //TODO
	return true;
}

bool IPKFTP::ClientConnect(std::string host, std::string port)
{
	//Possible Improvement: std::cout logging
	if (tcp.IsConnected()) {
		tcp.Close();
	}
	for (int i = 1; i <= retries; i++) {
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
				if (i >= retries) throw;
			}
			else {
				throw;
			}
		}
		catch (const IPKPacketException &e) {
			if (e.error == SignatureError || e.error == VersionError || e.error == TransmissionTypeError || 
				e.error == SizeError || e.error == CRC32Error) {
				if (i >= retries) throw;
			} else {
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

bool IPKFTP::Upload(std::string filename) // TODO: NOW
{
	//Possible Improvement: std::cout logging
	//Possible Improvement: split large files

	auto filedata = FileLoad(filename); // TODO: catch exception 

	for (int i = 1; i <= retries; i++) {
		try {
			tcp.Send(IPKPacket(OfferFile, filename, filedata));
			auto status = IPKPacket(tcp.Recv(IPKPacket::StatusSize));
			if (status == StatusOk) {
				return true;
			} 
			else {
				continue;
			}
		}
		catch (const TCPException &e) {
			if (e.error == ConnectionClosed || e.error == Timeout || e.error == ConnectFailed) {
				if (i >= retries) throw;
			}
			else {
				throw;
			}
		}
		catch (const IPKPacketException &e) {
			if (e.error == SignatureError || e.error == VersionError || e.error == TransmissionTypeError ||
				e.error == SizeError || e.error == CRC32Error) {
				if (i >= retries) throw;
			}
			else if (e.error == PacketCreationError) {
				std::cerr << e.what();
				return false;
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
	try {
		tcp.Send(IPKPacket(RequestFile, filename));
		auto packet = tcp.Recv(IPKPacket::StatusSize);
		tcp.Recv(packet, IPKPacket::ExpectedSize(packet) - IPKPacket::StatusSize);
		IPKPacket p(packet); //TODO: test filename == p.GetFilename() throw runtime exception, retry
		FileSave(p.GetFilename(), p.GetData());  // TODO: catch exception 
		tcp.Send(IPKPacket(StatusOk));
	}
	catch (const TCPException &e) {
		throw;
		return false;
	}
	return true;
}

bool IPKFTP::Upload(std::string host, std::string port, std::string filename)
{
	ClientConnect(host, port);
	auto ret = Upload(filename);
	ClientDisconnect();
	return ret;
}

bool IPKFTP::Download(std::string host, std::string port, std::string filename)
{
	ClientConnect(host, port);
	auto ret = Download(filename);
	ClientDisconnect();
	return ret;
}
