#ifndef LOGS_HPP
#define LOGS_HPP

#include <string>

#define print_error(msg) Log::log(LOG_TYPE::ERR,\
								  std::string(__FILE__) + "," + std::string(__PRETTY_FUNCTION__) + "," + std::to_string(__LINE__), msg)

enum LOG_TYPE {
	INFO = 0,
	WARN = 1,
	ERR  = 2,
	UNIMPORTANT = 4
};

// for now uses print, in the future should be singleton and log to files
struct Log {
	Log() = delete;

	static void log(const std::string &text);
	static void log(const std::string &title, const std::string &text);
	static void log(LOG_TYPE type, const std::string &text);
	static void log(LOG_TYPE type, const std::string &title, const std::string &text);
};

#endif
