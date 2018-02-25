/*
*	IPK Project 1: client-server for simple file transfer
*	Author: Tomáš Pazdiora (xpazdi02)
*	File: CRC32.h
*/

#ifndef CRC32_H
#define CRC32_H

#include <vector>
#include <stdint.h>

uint32_t CRC32(const std::vector<unsigned char>::const_iterator begin, const std::vector<unsigned char>::const_iterator end);

#endif