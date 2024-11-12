#ifndef IMAGE_H
#define IMAGE_H

#include <string>

// copied from stb:
// enum
// {
//    STBI_default = 0, // only used for desired_channels

//    STBI_grey       = 1,
//    STBI_grey_alpha = 2,
//    STBI_rgb        = 3,
//    STBI_rgb_alpha  = 4
// };

enum CHANNELS {
	DEFAULT = 0,
	GREY = 1,
	GREY_ALPHA = 2,
	RGB = 3,
	RGBA = 4
};

class Image {
public:
	Image() = delete;
	Image(const std::string &path, CHANNELS channels = CHANNELS::RGBA, bool flip = true); // I assume compiler sorts this out
	// will auto resize
	Image(const std::string &path, int desired_width, int desired_height, CHANNELS channels = CHANNELS::RGBA);
	~Image();

	constexpr int width() const {
		return _width;
	}
	constexpr int height() const {
		return _height;
	}
	constexpr int BPP() const {
		return _BPP;
	}
	constexpr const unsigned char* buffer() const {
		return _buffer;
	}
	void resize_to(int width, int height);
private:
	int _width, _height, _BPP;
	unsigned char *_buffer;
	CHANNELS channels;
};

#endif
