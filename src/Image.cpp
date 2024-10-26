#include "Image.hpp"

#include "TextureArray.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize2.h"

#include "Crash.hpp"
#include "Logs.hpp"

// TODO when loading it only cares about num channels, when resizing it cares about their layot, wtf???? abstract it better
stbir_pixel_layout channels_to_layout(CHANNELS channels) {
	// this enum is used for resizing:
	/*
	typedef enum
{
  STBIR_1CHANNEL = 1,
  STBIR_2CHANNEL = 2,
  STBIR_RGB      = 3,               // 3-chan, with order specified (for channel flipping)
  STBIR_BGR      = 0,               // 3-chan, with order specified (for channel flipping)
  STBIR_4CHANNEL = 5,

  STBIR_RGBA = 4,                   // alpha formats, where alpha is NOT premultiplied into color channels
  STBIR_BGRA = 6,
  STBIR_ARGB = 7,
  STBIR_ABGR = 8,
  STBIR_RA   = 9,
  STBIR_AR   = 10,

  STBIR_RGBA_PM = 11,               // alpha formats, where alpha is premultiplied into color channels
  STBIR_BGRA_PM = 12,
  STBIR_ARGB_PM = 13,
  STBIR_ABGR_PM = 14,
  STBIR_RA_PM   = 15,
  STBIR_AR_PM   = 16,

  STBIR_RGBA_NO_AW = 11,            // alpha formats, where NO alpha weighting is applied at all!
  STBIR_BGRA_NO_AW = 12,            //   these are just synonyms for the _PM flags (which also do
  STBIR_ARGB_NO_AW = 13,            //   no alpha weighting). These names just make it more clear
  STBIR_ABGR_NO_AW = 14,            //   for some folks).
  STBIR_RA_NO_AW   = 15,
  STBIR_AR_NO_AW   = 16,

} stbir_pixel_layout;
	*/

	switch(channels) {
		case CHANNELS::GREY:
			return STBIR_1CHANNEL;
			break;
		case CHANNELS::GREY_ALPHA:
			return STBIR_2CHANNEL;
			break;
		case CHANNELS::RGB:
			return STBIR_RGB;
			break;
		case CHANNELS::RGBA:
			return STBIR_RGBA;
			break;
		default:
			return STBIR_RGBA;
			break;
	}
}

Image::Image(const std::string &path, CHANNELS channels)
: channels(channels)
{
	stbi_set_flip_vertically_on_load(true);
	_buffer = stbi_load(path.c_str(), &_width, &_height, &_BPP, channels); // 4 -> RGBA or just use STBI_rgb_alpha

	CRASH_IF(!_buffer, "Error loading image");

	// if (_BPP != 4) {
	// 	Log::log(LOG_TYPE::WARN, std::string(__PRETTY_FUNCTION__), std::string(path) + " is not RGBA, BPP is " + std::to_string(_BPP));
	// }
}

Image::Image(const std::string &path, int desired_width, int desired_height, CHANNELS channels)
: channels(channels)
{
	stbi_set_flip_vertically_on_load(true);
	_buffer = stbi_load(path.c_str(), &_width, &_height, &_BPP, channels); // 4 -> RGBA or just use STBI_rgb_alpha

	CRASH_IF(!_buffer, "Error loading image: " + path);

	// if (_BPP != 4) {
	// 	Log::log(LOG_TYPE::WARN, std::string(__PRETTY_FUNCTION__), std::string(path) + " is not RGBA, BPP is " + std::to_string(_BPP));
	// }

	if (_width != desired_width || _height != desired_height) {
		Log::log(LOG_TYPE::WARN, "image dimensions for " + std::string(path) + ": Expected " + std::to_string(desired_width) + " " + std::to_string(desired_height) + 
			" got " + std::to_string(_width) + " " + std::to_string(_height) + ". The image will be automatically resized");

		resize_to(desired_width, desired_height);
	}
}

Image::~Image() {
	STBI_FREE(_buffer);
}

void Image::resize_to(int newwidth, int newheight) {
	unsigned char * resized_buffer = (unsigned char *)STBI_MALLOC(newwidth * newheight * channels);


	stbir_resize_uint8_linear(_buffer, _width, _height, 0, resized_buffer, newwidth, newheight, 0, channels_to_layout(channels));

	STBI_FREE(_buffer);
	_buffer = resized_buffer;
	_width = newwidth;
	_height = newheight;
}
