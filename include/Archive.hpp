#ifndef ARCHIVE_H
#define ARCHIVE_H

#include "types.hpp"
#include "stdlib.h"
#include "CustomVec.hpp"
#include <entt.hpp>
#include "Components.hpp"
#include "Files.hpp"




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
	CustomArchive(FileHandler &file);
	~CustomArchive() = default;

	template<typename T>
	void serializeIntoBuffer(const T &t);

	// registry should already exist, so I made this exceptionally
	void deSerializeIntoRegistry(entt::registry &registry) const;

	// TODO!!!!!!!!!!!!! this will call copy constructor, how to prevent this??
	template<typename T>
	T deSerializeFromBuffer();

	const CustomVec<uint8_t> &getData() { return data; }
	size_t read_sp = 0; // a hack since I need to know where I am when reading


private:
	CustomVec<uint8_t> data;
};

// template<>
// void CustomArchive::serializeIntoBuffer<uint32_t>(uint32_t &val);

// template<>
// void CustomArchive::serializeIntoBuffer<glm::vec3>(glm::vec3 &vec);

// template<>
// void CustomArchive::serializeIntoBuffer<Position>(Position &pos);

// template<>
// void CustomArchive::serializeIntoBuffer<entt::registry>(entt::registry &registry);

#endif