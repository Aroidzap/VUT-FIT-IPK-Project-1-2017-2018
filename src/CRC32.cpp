/*
*	IPK Project 1: client-server for simple file transfer
*	Author: Tomáš Pazdiora (xpazdi02)
*	File: CRC32.cpp
*/

#include "CRC32.h"
#include <array>

// C++14 (extended constexpr) required !!!

static constexpr uint32_t polynomial = crc32_polynomial;

struct CRC32LUT
{
	uint32_t values[0x100];

	constexpr uint32_t& operator[](size_t i)
	{
		return values[i];
	}

	constexpr const uint32_t& operator[](size_t i) const
	{
		return values[i];
	}
};

// bit reversal
static constexpr uint32_t bit_reverse(const uint32_t x) {
	uint32_t reversed{ 0 };
	for (uint32_t i = 0; i < 32; i++) {
		reversed |= ((x >> i) & 1) << (31 - i);
	}
	return reversed;
}

// function for compiling CRC32 lookup table (256 entries)
static constexpr CRC32LUT crc32lut_compile(const uint32_t polynomial)
{
	CRC32LUT lut{};

	const uint32_t reversed_polynomial = bit_reverse(polynomial);

	for (unsigned int i = 0; i <= 0xFF; i++)
	{
		uint32_t crc = i;
		for (unsigned int j = 0; j < 8; j++) {
			crc = (crc >> 1) ^ (-int(crc & 1) & reversed_polynomial);
		}
		lut[i] = crc;
	}
	return lut;
}

// CRC32 lookup table (256 entries)
static constexpr CRC32LUT crc32lut{ crc32lut_compile(polynomial) };

// CRC32 with lookup table
uint32_t CRC32(const std::vector<unsigned char>::const_iterator begin, const std::vector<unsigned char>::const_iterator end)
{
	uint32_t crc = 0xFFFFFFFF;
	for (auto it = begin; it != end; it++) {
		crc = (crc >> 8) ^ crc32lut[(crc & 0xFF) ^ *it];
	}
	return ~crc;
}