#include "image.h"

int main() {
	using namespace vivid;
	using namespace util;
	
	Image image("cartoon_goat.png");
	LOG("Size: " << image.getWidth() << "x" << image.getHeight());
	const Pixel* pixels = image.getPixels();

	int x = 0;
	int y = 0;
	Pixel pixel = pixels[x + y * image.getWidth()];
	std::printf("%08X\n", pixel.color());
	
	return 0;
}
