#include "image.h"

#define LOG(x) std::cout << x << std::endl

int main() {
	using namespace vivid;
	using namespace util;
	
	Image image("res/cartoon_goat.png");
	LOG("Size: " << image.getWidth() << "x" << image.getHeight());
	LOG("Color format: " << image.getFormat().colorFormat);
	LOG("Bit depth: " << image.getFormat().bitDepth);
	LOG("Compression method: " << image.getFormat().compressionMethod);
	LOG("Filter method: " << image.getFormat().filterMethod);
	LOG("Interlace method: " << image.getFormat().interlaceMethod);
	//const PixelRGBA* pixels = (PixelRGBA*) image.getData();

	unsigned int x = 29;
	unsigned int y = 9;
	PixelRGBA pixel = image.getPixel(x, y);
	std::printf("%08X\n", pixel.color());
	
	return 0;
}
