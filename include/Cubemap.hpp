#ifndef CUBEMAP_H
#define CUBEMAP_H

#include "Image.hpp"
#include "common.hpp"


class Cubemap {
	public:
		Cubemap() = delete;
		// IMAGES ARE EXPECTED AS RGB!!! BPP = 3!!!!!!!!
		Cubemap(const std::array<Image, 6> &images);
		~Cubemap();

		void bindCubemapToSlot(const GLuint slot) const;

		GLuint ID; // texture ID
};

#endif
