/*
*	IPK Project 1: client-server for simple file transfer
*	Author: Tomáš Pazdiora (xpazdi02)
*	File: IPKPacket.h
*/

#ifndef IPKPACKET_H
#define IPKPACKET_H

/**************** IPKPacket **************
*
*  offset |   size   |       name
*  ---------------------------------------
*  0h     | 6 bytes  | signature "IPKFTP"
*  6h     | 1 byte   | IPKFTP version
*  7h     | 1 byte   | IPKTransmissionType
*  8h     | 8 bytes  | overall message size
*
*  10h    | optional | filename (null terminated)
*  ???    | optional | file data
*
*  end-4h | 4 bytes  | message CRC32
*  ---------------------------------------
*
*  *the "overall message size" is complete size of message including CRC32
*  *CRC32 is computed for "overall message size" minus 4 bytes
*  *file size can be determined using "overall message size"
*
************ IPKTransmissionType *********
*
* (0) RequestFile - requires filename
* (1) OfferFile - requires filename and data
* (2) CommandPing
* (3) StatusOk
* (4) StatusError
* (5) *reserved* StatusFileBusy
* (6) *reserved* StatusFileAlreadyExists
* (7) *reserved* CommandOverwrite
* (8) *reserved* CommandCancelTransmission
*
******************************************/

#include <string>
#include <vector>
#include <stdexcept>

enum IPKTransmissionType {
	RequestFile = 0,
	OfferFile = 1,
	CommandPing = 2,
	StatusOk = 3,
	StatusError = 4,
	/*reserved*/ StatusFileBusy = 5,
	/*reserved*/ StatusFileAlreadyExists = 6,
	/*reserved*/ CommandOverwrite = 7,
	/*reserved*/ CommandCancelTransmission = 8,
	IPKUnknown = 9
};

enum IPKPacketError {
	PacketCreationError,
	SignatureError,
	VersionError,
	TransmissionTypeError,
	SizeError,
	CRC32Error
};

class IPKPacket {
	static const std::string signature;
	static const uint8_t version;
	const IPKTransmissionType type;
	const std::string filename;
	const std::vector<unsigned char> data;
public:
	// Create Packet
	IPKPacket(IPKTransmissionType type, std::string filename = {}, std::vector<unsigned char> data = {});

	// Deserialize
	IPKPacket(const std::vector<unsigned char> message);

	// Serialize
	operator const std::vector<unsigned char>() const;

	// Get Filename, Data, type
	const std::string GetFilename() const;
	const std::vector<unsigned char> GetData() const;
	const IPKTransmissionType Type() const;

	// Get type from incomplete serialized packet (min size == 8)
	static IPKTransmissionType Type(const std::vector<unsigned char> message);

	// Get expected size from incomplete serialized packet (min size == 16)
	static std::size_t ExpectedSize(const std::vector<unsigned char> message);
	static const std::size_t StatusSize;

	// Comparison
	bool operator==(const IPKTransmissionType t) const;
	bool operator!=(const IPKTransmissionType t) const;
};

class IPKPacketException : public std::runtime_error {
public:
	const IPKPacketError error;
	IPKPacketException(const IPKPacketError error, const std::string message = "IPKPacketError");
};

#endif
