#include "image.h"

int main() {
	Image image("squiggles.png");
	LOG("Size: " << image.getWidth() << "x" << image.getHeight());
	const Pixel* pixels = image.getPixels();

	int x = 21;
	int y = 5;
	Pixel pixel = pixels[x + y * image.getWidth()];
	std::printf("%08X\n", pixel.color());
	
	return 0;
}
