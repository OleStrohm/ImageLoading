//
// Created by Ole on 10.09.2017.
//

#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <utility>
#include <bitset>

unsigned char reversed(unsigned char value);

#define LOG(x) (std::cout << x << std::endl)
#define LOGB(x) (std::cout << (std::bitset<8>)reversed(x) << std::endl)

//unsigned int crc_table[256];
//
//void build_crc32() {
//	for (unsigned int n = 0; n < 256; n++) {
//		auto c      = n;
//		for (unsigned int k = 0; k < 8; k++) {
//			if (c & 1)
//				c = (c >> 1) ^ 0xedb88320;
//			else
//				c = c >> 1;
//		}
//		crc_table[n] = c;
//	}
//}
//
//void getCRC() {
//	LOG(crc_table[0]);
//}

struct Chunk {
	unsigned int length;
	std::string type;
	std::vector<unsigned char> data;
	unsigned int crc;
	
	Chunk(unsigned int length, std::string type, std::vector<unsigned char>& data, unsigned int crc)
			: length(length), type(std::move(type)), data(data), crc(crc) {}
};

struct Format {
	unsigned int width;
	unsigned int height;
	char bitDepth;
	char colorType;
	char compressionMethod;
	char filterMethod;
	char interlaceMethod;
	
	Format()
			: width(0),height(0),bitDepth(0),colorType(0),compressionMethod(0),filterMethod(0),interlaceMethod(0) {}
};

class Image {
public:
	std::vector<Chunk*> chunks;
private:
	Format format;
public:
	Image(const std::string& path);
	
	void init(const std::string& path);
	
	void formatIHDR();
	std::vector<unsigned char> extractCompressedPixelData(const Chunk& chunk);
	void loadChunks(const unsigned char* data, unsigned int size);
	
	inline const Format& getFormat() const {
		return format;
	}
private:
	Chunk* loadChunk(const unsigned char* data, unsigned int& offset);
};


