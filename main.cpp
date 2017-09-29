#include <cstring>
#include <sstream>
#include <cmath>
#include "image.h"
#include "bitarray.h"
#include "tree.h"

static const unsigned short huffmanTreeOrder[19] = {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};
static const unsigned short lengths[29] = {3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31, 35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258};
static const unsigned short lengthsExtraBits[29] = {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0};
static const unsigned int offsets[30] = {1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193, 257, 385, 513, 769, 1025, 1537, 2049, 3073, 4097, 6145, 8193, 12289, 16385, 24577};
static const unsigned int offsetsExtraBits[30] = {0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13};

int main() {
	Image image("squiggles.png");
	
	Format format = image.getFormat();
	LOG("Size: " << format.width << "x" << format.height);
	LOG("Bit depth: " << (int) format.bitDepth);
	LOG("Color type: " << (int) format.colorType);
	LOG("Compression method: " << (int) format.compressionMethod);
	LOG("Filter method: " << (int) format.filterMethod);
	LOG("Interlace method: " << (int) format.interlaceMethod);
	LOG("");
	
	unsigned int firstIDAT = 0;
	while (image.chunks[++firstIDAT]->type != "IDAT");
	unsigned int lastIDAT = firstIDAT;
	while (++lastIDAT < image.chunks.size() && image.chunks[lastIDAT]->type == "IDAT");
	Chunk pixelChunk = *image.chunks[firstIDAT];
	
	std::vector<unsigned char> firstCompressedData = image.extractCompressedPixelData(pixelChunk);
	BitArray CMFbits(firstCompressedData[0], 8);
	LOG("CMF:");
	LOG("\tHopefully this is 8: " << CMFbits.read(0, 4));
	LOG("\tCINFO: " << CMFbits.read(4, 4));
	
	BitArray flagBits(firstCompressedData[1], 8);
	LOG("FLG:");
	bool fDict = flagBits.read(5, 1) != 0;
	LOG("\tFDICT: " << (fDict ? "yep" : "no"));
	LOG("\tChecks out: " << ((((CMFbits.read(0, 8) << 8) + (flagBits.read(0, 8))) % 31 == 0) ? "yeeaaah" : "no... :/"));
	LOG("");
	
	std::vector<std::string> fixedTreeSymbols;
	fixedTreeSymbols.reserve(288);
	std::vector<unsigned int> fixedTreeLengths;
	fixedTreeLengths.reserve(288);
	for (unsigned int i = 0; i < 288; i++) {
		fixedTreeSymbols.push_back(std::to_string(i));
		if (i <= 143)
			fixedTreeLengths.push_back(8);
		else if (i <= 255)
			fixedTreeLengths.push_back(9);
		else if (i <= 279)
			fixedTreeLengths.push_back(7);
		else
			fixedTreeLengths.push_back(8);
	}
	Tree fixedTree(fixedTreeSymbols, fixedTreeLengths);
	
	LOG("");
	unsigned int end = 0;
	BitArray bitStream;
	for (unsigned int i = 2; i < firstCompressedData.size(); i++)
		bitStream.pushBack(firstCompressedData[i]);
	
	for (unsigned int index = firstIDAT + 1; index <= lastIDAT; index++) {
		std::vector<unsigned char> compressedData = image.extractCompressedPixelData(*(image.chunks[index]));
		for (unsigned char i : compressedData)
			bitStream.pushBack(i);
	}
	
	std::vector<unsigned int> dataStream;
	dataStream.reserve((1 + format.width) * format.height);
	
	bool lastBlock = false;
	while (!lastBlock) {
		lastBlock = bitStream.read(end, 1) == 1;
		end += 1;
		unsigned short compressionMethod = (unsigned short) bitStream.read(end, 2);
		end += 2;
		
		if (compressionMethod == 1) {
			while (true) {
				std::stringstream convert(fixedTree.uncompressOneCode(bitStream, end, &end));
				
				unsigned int code = 0;
				convert >> code;
				
				if (code < 256) {
					dataStream.emplace_back(code);
				} else if (code == 256) {
					break;
				} else {
					code -= 257;
					unsigned int length = lengths[code];
					length += bitStream.read(end, lengthsExtraBits[code]);
					end += lengthsExtraBits[code];
					
					unsigned int offsetCode = bitStream.read(end, 5, true);
					end += 5;
					unsigned int offset = offsets[offsetCode];
					offset += bitStream.read(end, offsetsExtraBits[offsetCode]);
					end += offsetsExtraBits[offsetCode];

					for (int i = 0; i < length; i++) {
						dataStream.emplace_back(dataStream[dataStream.size() - offset]);
					}
				}
			}
		} else if (compressionMethod == 2) {
			unsigned int hlit = bitStream.read(end, 5, false) + 257;
			end += 5;
			unsigned int hdist = bitStream.read(end, 5, false) + 1;
			end += 5;
			unsigned int hlen = bitStream.read(end, 4, false) + 4;
			end += 4;
			
			std::vector<std::string> huffman1Symbols;
			huffman1Symbols.reserve(19);
			std::vector<unsigned int> huffman1Lengths(19);
			{
				unsigned int i = 0;
				for (; i < hlen; i++) {
					huffman1Symbols.push_back(std::to_string(i));
					huffman1Lengths[huffmanTreeOrder[i]] = bitStream.read(end, 3, false);
					end += 3;
				}
				for (; i < 19; i++) {
					huffman1Symbols.push_back(std::to_string(i));
					huffman1Lengths[huffmanTreeOrder[i]] = 0;
				}
			}
			Tree huffmanTree1(huffman1Symbols, huffman1Lengths);
			
			std::vector<unsigned int> litAndDistLengths;
			litAndDistLengths.reserve(hlit + hdist);
			
			while (litAndDistLengths.size() < hlit + hdist) {
				std::stringstream convert(huffmanTree1.uncompressOneCode(bitStream, end, &end));
				unsigned int code = 0;
				convert >> code;
				
				if (code <= 15) {
					litAndDistLengths.push_back(code);
				} else if (code == 16) {
					unsigned int extra = bitStream.read(end, 2, false);
					end += 2;
					
					unsigned long long int copyPos = litAndDistLengths.size() - 1;
					for (unsigned int i = 0; i < 3 + extra; i++) {
						litAndDistLengths.push_back(litAndDistLengths[copyPos]);
					}
				} else if (code == 17) {
					unsigned int extra = bitStream.read(end, 3, false);
					end += 3;
					
					for (unsigned int i = 0; i < 3 + extra; i++) {
						litAndDistLengths.emplace_back(0);
					}
				} else/*if (code == 18)*/{
					unsigned int extra = bitStream.read(end, 7, false);
					end += 7;
					
					for (unsigned int i = 0; i < 11 + extra; i++) {
						litAndDistLengths.emplace_back(0);
					}
				}
			}
			
			std::vector<std::string> litSymbols;
			litSymbols.reserve(hlit);
			std::vector<std::string> distSymbols;
			distSymbols.reserve(hdist);
			
			for (unsigned int i = 0; i < hlit; i++)
				litSymbols.emplace_back(std::to_string(i));
			for (unsigned int i = 0; i < hdist; i++)
				distSymbols.emplace_back(std::to_string(i));
			
			std::vector<unsigned int> litLengths;
			litLengths.reserve(hlit);
			std::vector<unsigned int> distLengths;
			distLengths.reserve(hdist);
			for (unsigned int i = 0; i < hlit; i++)
				litLengths.emplace_back(litAndDistLengths[i]);
			for (unsigned int i = 0; i < hdist; i++)
				distLengths.emplace_back(litAndDistLengths[hlit + i]);
			
			Tree literalTree(litSymbols, litLengths);
			Tree distTree(distSymbols, distLengths);
			
			while (true) {
				std::stringstream convertCode(literalTree.uncompressOneCode(bitStream, end, &end));
				unsigned int code = 0;
				convertCode >> code;
				
				if (code < 256) {
					dataStream.push_back(code);
				} else if (code == 256) {
					break;
				} else {
					code -= 257;
					unsigned int length = lengths[code];
					length += bitStream.read(end, lengthsExtraBits[code]);
					end += lengthsExtraBits[code];
					
					std::stringstream convertOffsetCode(distTree.uncompressOneCode(bitStream, end, &end));
					unsigned int offsetCode = 0;
					convertOffsetCode >> offsetCode;
					
					unsigned int offset = offsets[offsetCode];
					offset += bitStream.read(end, offsetsExtraBits[offsetCode], false);
					end += offsetsExtraBits[offsetCode];

					for (int i = 0; i < length; i++) {
						dataStream.push_back(dataStream[dataStream.size() - offset]);
					}
				}
			}
		} else/*if (compressionMethod == 0)*/{
			unsigned int offsetFromByte = end & 0x7;
			if (offsetFromByte != 0)
				end += (8 - offsetFromByte);
			
			unsigned int length = bitStream.read(end, 16);
			end += 32;
			
			for(unsigned int i = 0; i < length; i++) {
				dataStream.emplace_back(bitStream.read(end, 8));
				end += 8;
			}
		}
	}
	
	
	std::vector<unsigned int> imageArray;
	imageArray.reserve(format.width * format.height);
	
	for (int y = 0; y < format.height; y++) {
		for (int x = 0; x < format.width; x++) {
			int i = 1 + 4 * x + y * (1 + format.width * 4);
			imageArray.push_back(dataStream[i]);
			imageArray.push_back(dataStream[i + 1]);
			imageArray.push_back(dataStream[i + 2]);
			imageArray.push_back(dataStream[i + 3]);
		}
	}
	
	int x = 21;
	int y = 5;
	
	unsigned int r = imageArray[y * format.width * 4 + 4 * x];
	unsigned int g = imageArray[y * format.width * 4 + 4 * x + 1];
	unsigned int b = imageArray[y * format.width * 4 + 4 * x + 2];
	unsigned int a = imageArray[y * format.width * 4 + 4 * x + 3];
	unsigned int col = a << 24 | r << 16 | g << 8 | b;
	std::printf("%08X", col);
//	unsigned int col = r << 16 | g << 8 | b;
//	std::printf("%06X", col);
	
	LOG("\nDONE!");
	
	return 0;
}
