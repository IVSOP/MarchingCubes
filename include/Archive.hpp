#ifndef ARCHIVE_H
#define ARCHIVE_H

#include "types.hpp"
#include "stdlib.h"
#include "CustomVec.hpp"
#include <entt.hpp>
#include "Components.hpp"
#include "Files.hpp"
#include "Compression.hpp"




/*
this is a mess i know
for whatever reason this is what I decided:

entites can be serialized with this, by passing in the entire registry
however, physics bodies are not so easy to serialize
so, the render_id will be used to load their bodies as needed (outside this class)
*/





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

private:
	CustomVec<uint8_t> data;
};

#endif