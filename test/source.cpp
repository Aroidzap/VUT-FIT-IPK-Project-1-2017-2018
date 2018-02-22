#include <iostream>
#include "../src/IPKPacket.h"
#include "../src/IPKFTP.h"

int main() {

	auto filename = "source.cpp";

	IPKFTP ipkftp;
	auto data = ipkftp.FileLoad(filename);

	IPKPacket packet1(OfferFile, filename, data);

	std::vector<unsigned char> s1 = packet1;

	try {
		IPKPacket packet2(s1);
	}
	catch(IPKPacketException &e){
		std::cout << e.what();
	}
	

	return 0;
}