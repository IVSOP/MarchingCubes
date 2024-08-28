#include "TextureArray.hpp"
#include "Crash.hpp"
#include "Logs.hpp"
#include "Image.hpp"

TextureArray::TextureArray(GLsizei _width, GLsizei _height, GLsizei _depth)
: width(_width), height(_height), depth(_depth), sp(0)
{
	GLsizei numMipmaps = 3;
	GLCall(glGenTextures(1, &this->ID));
	GLCall(glBindTexture(GL_TEXTURE_2D_ARRAY, this->ID));

												// 3 + base level
	GLCall(glTexStorage3D(GL_TEXTURE_2D_ARRAY, numMipmaps + 1, GL_RGBA8, _width, _height, _depth)); // allocate storage for texture
	// uncommenting code below also works, idk if mipmaps work though
	// GLCall(glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA, _width, _height, _depth, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr));

	GLCall(glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
	GLCall(glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
	// GLCall(glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
	// GLCall(glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

	GLCall(glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE)); // BORDER???
	GLCall(glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE)); // BORDER???
}

// is this all that is needed??
TextureArray::~TextureArray() {
	GLCall(glBindTexture(GL_TEXTURE_2D_ARRAY, 0));
	GLCall(glDeleteTextures(1, &ID));
}

// texture array needs to be bound but not necessarily activated
void TextureArray::addTexture(const std::string &path) {
	CRASH_IF(this->sp >= this->depth - 1, "sp exceeds max depth. This class is not prepared to handle such cases, change it to have multiple texture arrays and manage them or something");

	Image image = Image(path, this->width, this->height);

	constexpr GLint LOD = 0,
	xoffset = 0,
	yoffset = 0;
	// Z-offset is used to place the image in the correct place
	constexpr GLsizei depth = 1; 
	// depth is actually if the image were 3D. it isnt so I just set it to 1
	GLCall(glTexSubImage3D(GL_TEXTURE_2D_ARRAY, LOD, xoffset, yoffset, this->sp, image.width(), image.height(), depth, GL_RGBA, GL_UNSIGNED_BYTE, image.buffer()));
	// IMPORTANT this is very bad since it generates mipmaps for ALL the textures, but since they are not changed at runtime it's not too bad (for now)
	GLCall(glGenerateMipmap(GL_TEXTURE_2D_ARRAY));

	this->sp ++;
}

void TextureArray::setTextureArrayToSlot(const GLuint slot) {
	GLCall(glActiveTexture(GL_TEXTURE0 + slot));
	GLCall(glBindTexture(GL_TEXTURE_2D_ARRAY, ID));
}
