/*
*	IPK Project 1: client-server for simple file transfer
*	Author: Tomáš Pazdiora (xpazdi02)
*	File: IPKFTP.cpp
*/

#include "IPKFTP.h"

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
