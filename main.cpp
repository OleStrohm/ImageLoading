
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
	Image image("smile.png");

	Format format = image.getFormat();
	LOG("Size: " << format.width << "x" << format.height);
	LOG("Bit depth: " << (int) format.bitDepth);
	LOG("Color type: " << (int) format.colorType);
	LOG("Compression method: " << (int) format.compressionMethod);
	LOG("Filter method: " << (int) format.filterMethod);
	LOG("Interlace method: " << (int) format.interlaceMethod);
	LOG("");

	int index = 0;
	int count = 0;
	while (image.chunks[++index]->type != "IDAT");
	int index2 = index;
	while (index < image.chunks.size() && image.chunks[index2++]->type == "IDAT") count++;
	LOG("IDAT chunk count: " << count);
	Chunk pixelChunk = *image.chunks[index];
	LOG("Chunk type: " << pixelChunk.type);
	LOG("Chunk size (bytes): " << pixelChunk.length);
	LOG("");

	std::vector<unsigned char> compressedData = image.extractCompressedPixelData(pixelChunk);
	BitArray CMFbits(compressedData[0], 8);
	LOG("CMF:");
	LOG("\tHopefully this is 8: " << CMFbits.read(0, 4));
	LOG("\tCINFO: " << CMFbits.read(4, 4));

	BitArray flagBits(compressedData[1], 8);
	LOG("FLG:");
	bool fDict = flagBits.read(5, 1) != 0;
	LOG("\tFDICT: " << (fDict ? "yep" : "no"));
	LOG("\tChecks out: " << ((((CMFbits.read(0, 8) << 8) + (flagBits.read(0, 8))) % 31 == 0) ? "yeeaaah" : "no... :/"));
	LOG("");

	BitArray nextBits(compressedData[2], 8);
	LOG("Last block?: " << (nextBits.read(0, 1) ? "yes" : "no"));
	unsigned short compressionMethod = (unsigned short) nextBits.read(1, 2);
	switch (compressionMethod) {
		case 0:
			LOG("Compression method: uncompressed");
			break;
		case 1:
			LOG("Compression method: fixed");
			break;
		case 2:
			LOG("Compression method: dynamic");
			break;
		default:
			LOG("Compression method: ERROR");
	}
	LOG("");
	BitArray actualData = BitArray(nextBits.read(3, 5), 5);
	for (unsigned int i = 3; i < compressedData.size(); i++) {
		actualData.pushBack(compressedData[i]);
	}

	LOG("\n");

	unsigned int end = 0;
	if (compressionMethod == 2) {
		unsigned int hlit = actualData.read(end, 5, true) + 257;
		end += 5;
		unsigned int hdist = actualData.read(end, 5, true) + 1;
		end += 5;
		unsigned int hlen = actualData.read(end, 5, true) + 4;
		end += 4;

		std::vector<std::string> huffman1Symbols;
		huffman1Symbols.reserve(19);
		std::vector<unsigned int> huffman1Lengths;
		huffman1Lengths.reserve(19);
		{
			unsigned int i = 0;
			for (; i < hlen; i++) {
				huffman1Symbols.push_back(std::to_string((unsigned char) i));
				huffman1Lengths[huffmanTreeOrder[i]] = (unsigned char) actualData.read(end, 3, false);
				end+=3;
			}
			for(; i < 19; i++) {
				huffman1Symbols.push_back(std::to_string((unsigned char) i));
				huffman1Lengths[huffmanTreeOrder[i]] = 0;
			}
		}

		Tree huffmanTree1(huffman1Symbols, huffman1Lengths);
		huffmanTree1.print();
	} else if (compressionMethod == 1) {

		std::vector<std::string> pngSymbols;
		pngSymbols.reserve(288);
		std::vector<unsigned int> pngLengths;
		pngLengths.reserve(288);
		for (unsigned int i = 0; i < 288; i++) {
			pngSymbols.push_back(std::to_string(i));
			if (i <= 143)
				pngLengths.push_back(8);
			else if (i <= 255)
				pngLengths.push_back(9);
			else if (i <= 279)
				pngLengths.push_back(7);
			else
				pngLengths.push_back(8);
		}
		Tree pngTree(pngSymbols, pngLengths);

		std::vector<unsigned int> stream;
		stream.reserve((1 + format.width) * format.height);

		while (true) {
			std::stringstream convert(pngTree.uncompressOneCode(actualData, end, &end));

			unsigned int code = 0;
			convert >> code;

			if (code < 256) {
				LOG(code);
				stream.emplace_back(code);
			} else if (code == 256) {
//			LOG("\n");
//			for (auto i : stream)
//				LOG(i);
				LOG("END");
				break;
			} else {
				code -= 257;
				unsigned int length = lengths[code];
				length += actualData.read(end, lengthsExtraBits[code]);
				end += lengthsExtraBits[code];

				unsigned int offsetCode = actualData.read(end, 5, true);
				end += 5;
				unsigned int offset = offsets[offsetCode];
				offset += actualData.read(end, offsetsExtraBits[offsetCode]);
				end += offsetsExtraBits[offsetCode];

				LOG("<" << length << ", " << offset << ">");
				for (int i = 0; i < length; i++) {
					stream.emplace_back(stream[stream.size() - offset]);
				}
			}
		}


#if 1
		std::vector<unsigned int> imageArray;
		imageArray.reserve(format.width * format.height);

		for (int y = 0; y < format.height; y++) {
			for (int x = 0; x < format.width; x++) {
				int i = 1 + 4 * x + y * (1 + format.width * 4);
				imageArray.push_back(stream[i]);
				imageArray.push_back(stream[i + 1]);
				imageArray.push_back(stream[i + 2]);
				imageArray.push_back(stream[i + 3]);
			}
		}

		LOG("\n");
		for (int y = 0; y < format.height; y++) {
			for (int x = 0; x < format.width; x++) {
				int i = (y * (format.width) + x) * 4;
				int r = imageArray[i];
				int g = imageArray[i + 1];
				int b = imageArray[i + 2];
				int a = imageArray[i + 3];

				int col = r << 16 | g << 8 | b;
//			int col = a << 24 | r << 16 | g << 8 | b;
				std::printf("%06X ", col);
			}
			std::printf("\n");
		}
#endif
	}
<<<<<<< HEAD
	
	std::cin.get();
=======

>>>>>>> 947717afa71e01193fef2a5e2699832968c979ce
	return 0;
}
