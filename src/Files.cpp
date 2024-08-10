#include "Files.hpp"
#include "Logs.hpp"
#include "Crash.hpp"

FileHandler::FileHandler(const std::string &path)
: file(path)
{
	CRASH_IF(!file.is_open(), ("Error opening file: " + path).c_str());
}

FileHandler::~FileHandler() {
	file.close();
}

json FileHandler::readjson() {
	return json::parse(this->file);
}
