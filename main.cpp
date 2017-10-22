#include "image.h"

#define LOG(x) std::cout << x << std::endl;

int main() {
	using namespace vivid;
	using namespace util;
	
	Image image("res/cartoon_goat.png");
	LOG("Size: " << image.getWidth() << "x" << image.getHeight());
	const PixelRGBA* pixels = (PixelRGBA*) image.getData();

	int x = 0;
	int y = 0;
	PixelRGBA pixel = pixels[x + y * image.getWidth()];
	std::printf("%08X\n", pixel.color());
	
	return 0;
}
