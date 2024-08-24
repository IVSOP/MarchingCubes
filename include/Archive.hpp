#ifndef ARCHIVE_H
#define ARCHIVE_H

#include "types.hpp"
#include "stdlib.h"
#include "CustomVec.hpp"
#include <entt.hpp>
#include "Components.hpp"
#include "Files.hpp"
#include "Compression.hpp"

// TODO consider using boost archives instead of doing everything by hand
// TODO look into entt snapshots, I couldn't be bothered so I did everything by hand even if it is slower
class CustomArchive {
public:
	CustomArchive() : data(1) {}
	// CustomArchive(FileHandler &file);
	~CustomArchive() = default;

	// serialize into internal buffer
	template<typename T>
	void serializeIntoBuffer(const T &t);

	constexpr CustomVec<uint8_t> &getData() { return data; }

	constexpr void clear() { data.clear(); }

	///////////// ????????????? why tf are these even templates instead of just overloads

	template<typename T>
	static void serializeIntoFile(FileHandler &file, T &t);

	template<typename T>
	static void deserializeFromFile(FileHandler &file, T &t);

	// cant make this constexpr due to the reinterpret_cast, wtf??????????
	void setBuffer(void *newdata, size_t len) {
		data.clear();
		data.reserve(len);
		data.free_internal_buff();

		data._data = reinterpret_cast<uint8_t *>(newdata);
		data._sp = len;
		read_sp = 0;
	}

	template<typename T>
	void deserializeFromBuffer(T &t);

private:
	CustomVec<uint8_t> data;
	size_t read_sp = 0; // TODO make a serialize archive and a deserialize archive
};

#endif