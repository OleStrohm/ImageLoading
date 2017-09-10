//
// Created by Ole on 10.09.2017.
//

#include "image.h"

unsigned char reversed(unsigned char value) {
	unsigned char t = value;
	for (int i = sizeof(value) * 8-1; i; i--)
	{
		value >>= 1;
		t <<= 1;
		t |= value & 1;
	}
	return t;
}

Image::Image(const std::string& path) {
	init(path);
}

void Image::init(const std::string& path) {
	std::ifstream stream(path, std::ifstream::in | std::ifstream::ate | std::ifstream::binary);
	auto size = (unsigned int) stream.tellg();
	if (!stream.good()) {
		LOG("FAILED TO OPEN FILE: " << path);
		size = 0;
	}
	stream.seekg(0, std::ifstream::beg);
	
	unsigned char data[size];
	
	unsigned int position = 0;
	char in;
	while (stream.get(in))
		data[position++] = (unsigned char) in;
	
	loadChunks(data, size);
	formatIHDR();
	
	stream.close();
}

void Image::formatIHDR() {
	auto& data = chunks[0]->data;
	
	format.width = (data[0] << 24) | (data[1] << 16) | (data[2] << 8) | (data[3]);
	format.height = (data[4] << 24) | (data[5] << 16) | (data[6] << 8) | (data[7]);
	format.bitDepth = data[8];
	format.colorType = data[9];
	format.compressionMethod = data[10];
	format.filterMethod = data[11];
	format.interlaceMethod = data[12];
}

std::vector<unsigned char> Image::extractCompressedPixelData(const Chunk& chunk) {
	std::vector<unsigned char> compressedData;
	compressedData.reserve(chunk.length);
	for(int i = 0; i < chunk.length; i++) {
		compressedData.push_back(chunk.data[i]);
	}
	return compressedData;
}

void Image::loadChunks(const unsigned char* data, unsigned int size) {
	unsigned int offset = 8;
	while(offset < size) {
		chunks.push_back(loadChunk(data, offset));
	}
}

Chunk* Image::loadChunk(const unsigned char* data, unsigned int& offset) {
	unsigned int chunkLength = (data[offset + 0] << 24) | (data[offset + 1] << 16) | (data[offset + 2] << 8) | (data[offset + 3]);
	offset += 4;
	
	char s[5];
	s[0] = data[offset + 0];
	s[1] = data[offset + 1];
	s[2] = data[offset + 2];
	s[3] = data[offset + 3];
	s[4] = 0;
	std::string chunkType(s);
	offset += 4;
	
	std::vector<unsigned char> chunkData;
	chunkData.reserve(chunkLength);
	
	for (int p = 0; p < chunkLength; p++) {
		chunkData.push_back(data[offset + p]);
	}
	offset += chunkLength;
	
	unsigned int chunkCRC = (data[offset + 0] << 24) | (data[offset + 1] << 16) | (data[offset + 2] << 8) | (data[offset + 3]);
	offset += 4;
	
	return new Chunk(chunkLength, chunkType, chunkData, chunkCRC);
}
