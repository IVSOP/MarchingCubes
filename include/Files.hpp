#ifndef FILES_H
#define FILES_H

#include <string>
#include <fstream>

#include "Json.hpp"

using FileMode = std::ios_base::openmode;

// these are binary flags, just do A | B | C | ...
struct FileModes {
	static constexpr FileMode Bin = std::ios::binary;
	static constexpr FileMode Append = std::ios::app;
// in	(input) Allow input operations on the stream.
// out	(output) Allow output operations on the stream.
	static constexpr FileMode Trunc = std::ios::trunc;
	static constexpr FileMode Write = std::fstream::out;
	static constexpr FileMode Read = std::fstream::in;
};


struct FileHandler {
	FileHandler() = delete;
	FileHandler(const std::string &path);
	FileHandler(const std::string &path, FileMode flags);
	~FileHandler();

	// reads line, stores it into buffer. buffer is reset at the start. '\' makes it so that next character is ignored
	// no error checking is performed at all, other than returning false on EOF
	json readjson(); // make it so that buffer is internal to this class????

	size_t write(void *data, size_t len);
	size_t read(void *buff, size_t len);


	std::fstream file;
};

#endif
