/*
*	IPK Project 1: client-server for simple file transfer
*	Author: Tomáš Pazdiora (xpazdi02)
*	File: IPKFTP.cpp
*/

#include "IPKFTP.h"

#include "IPKPacket.h"

#include <fstream>
#include <iterator>

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
	tcp.Listen(port, [](TCP client) 
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
	}); // infinite loop
	return true;
}

bool IPKFTP::ClientConnect(std::string host, std::string port)
{
	if (tcp.IsConnected()) {
		tcp.Close();
	}
	try {
		tcp.Connect(host, port);
		tcp.Send(IPKPacket(CommandPing));
		if (!(IPKPacket(tcp.Recv(IPKPacket::StatusSize)) == StatusOk)) {
			tcp.Close();
			return false;
		}
	}
	catch (TCPError &e) {
		throw e;
		return false;
	}
	return true;
}

void IPKFTP::ClientDisconnect()
{
	tcp.Close();
}

bool IPKFTP::Upload(std::string filename)
{
	try {
		tcp.Send(IPKPacket(OfferFile, filename, FileLoad(filename)));
		if (!(IPKPacket(tcp.Recv(IPKPacket::StatusSize)) == StatusOk)) {
			return false;
		}
	}
	catch (TCPError &e) {
		throw e;
		return false;
	}
	return true;
}

bool IPKFTP::Download(std::string filename)
{
	try {
		tcp.Send(IPKPacket(RequestFile, filename));
		auto packet = tcp.Recv(IPKPacket::StatusSize);
		tcp.Recv(packet, IPKPacket::ExpectedSize(packet) - IPKPacket::StatusSize);
		IPKPacket p(packet); //TODO: test filename == p.GetFilename()
		FileSave(p.GetFilename(), p.GetData());
		tcp.Send(IPKPacket(StatusOk));
	}
	catch (TCPError &e) {
		throw e;
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
