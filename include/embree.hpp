#ifndef EMBREE_H
#define EMBREE_H

#include "stdlib.hpp"
#include "types.hpp"
#include "Crash.hpp"
#include "Vertex.hpp"
#include "CustomVec.hpp"

#include <embree4/rtcore.h>
#include <iostream>
#include <vector>
#include <limits>

struct MarchingCubesObject {
	uint32_t len_x, len_y, len_z; // len of the grid dimentions
	// [y][z][x] grid of boolean values
	bool *corners;
	// std::vector<std::vector<std::vector<bool>>> corners;

	MarchingCubesObject(uint32_t len_x, uint32_t len_y, uint32_t len_z)
	: len_x(len_x), len_y(len_y), len_z(len_z) {
		corners = reinterpret_cast<bool *>(std::calloc(len_x * len_y * len_z, sizeof(bool)));
	}

	~MarchingCubesObject() {
		std::free(corners);
	}

	constexpr void set(uint32_t x, uint32_t y, uint32_t z, bool val) {
		corners[(x * len_y * len_z) + (z * len_y) + y] = val;
	}

	constexpr bool get(uint32_t x, uint32_t y, uint32_t z) const {
		return corners[(x * len_y * len_z) + (z * len_y) + y];
	}
};

// // Error handling callback function
// void errorFunction(void* userPtr, enum RTCError error, const char* str) {
//     CRASH_IF(error != RTC_ERROR_NONE, "Embree error: " + std::string(str));
// }

class EmbreeWrapper {
public:
	static void marcheCubes(MarchingCubesObject *obj, const CustomVec<ModelVertex> &verts, const std::vector<GLuint> &indices, uint32_t len_x, uint32_t len_y, uint32_t len_z);
};

#endif
