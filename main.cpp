
#include <cstring>
#include "image.h"

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

	std::vector<unsigned char> compressedData = image.extractCompressedPixelData(*image.chunks[index]);
	LOG("CMF:");
	LOGB(compressedData[0] + (unsigned char) (8 <<4));
	LOG("FLG:");
	LOGB(compressedData[1]); // 16-bit sum is 14415 % 31 == 0
	LOG("");
	LOGB(compressedData[2]);
	LOGB(compressedData[3]);
	LOGB(compressedData[4]);
	LOGB(compressedData[5]);
	LOGB(compressedData[6]);
	
	return 0;
}
