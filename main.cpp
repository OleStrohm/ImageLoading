
#include <cstring>
#include "image.h"
#include "bitarray.h"

static const unsigned short order[19] = {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15}; /* permutation of code lengths */

int main() {
	{
		uint32_t i=0x01020304;
		char le[4]={4, 3, 2, 1};
		char be[4]={1, 2, 3, 4};
		if(memcmp(&i, le, 4)==0)
			puts("Little endian");
		else if(memcmp(&i, be, 4)==0)
			puts("Big endian");
		else
			puts("Mixed endian");
	}
	
	Image image("smile.png");
	
	Format format = image.getFormat();
	LOG("bit depth: " << (int) format.bitDepth);
	LOG("color type: " << (int) format.colorType);
	LOG("compression method: " << (int) format.compressionMethod);
	LOG("filter method: " << (int) format.filterMethod);
	LOG("interlace method: " << (int) format.interlaceMethod);
	LOG("");

	int index = 0;
	while(image.chunks[++index]->type != "IDAT");
	Chunk pixelChunk = *image.chunks[index];
	LOG(pixelChunk.type);
	LOG(pixelChunk.length);
	LOG("");

	std::vector<unsigned char> compressedData = image.extractCompressedPixelData(pixelChunk);
	LOG("CMF:");
	BitArray CMFbits(compressedData[0], 8);
	LOG("\tHopefully this is 8: " << (unsigned int)CMFbits.read(0, 4, false));
	LOG("\tCINFO: " << (unsigned int)CMFbits.read(4, 4, false));
	LOG(CMFbits);
	
	BitArray flagBits(compressedData[1], 8);
	LOG("FLG: " << flagBits);
	LOG("\tFDICT:" << (unsigned int) flagBits.read(4, 1));
	LOG("");
	LOG("Checks out: " << (((CMFbits.read(0, 8) << 8) + (flagBits.read(0, 8))) % 31 == 0));
	LOG("");
	
	BitArray nextBits(compressedData[2], 8);
	LOG("Last block?: " << (unsigned int) nextBits.read(0, 1));
	switch(nextBits.read(1, 2)) {
		case 0:
			LOG("Compression Method: uncompressed");
			break;
		case 1:
			LOG("Compression Method: fixed");
			break;
		case 2:
			LOG("Compression Method: dynamic");
			break;
		default:
			LOG("Compression Method: ERROR");
	}
	LOG(nextBits);
	BitArray actualData(nextBits.read(3, 5), 5);
	actualData.pushBack(compressedData[3]);
	actualData.pushBack(compressedData[4]);
	actualData.pushBack(compressedData[5]);
	actualData.pushBack(compressedData[6]);
	LOG(actualData);
	
	return 0;
}
