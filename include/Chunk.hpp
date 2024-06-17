#ifndef CHUNK_H
#define CHUNK_H

#include <chrono>

#define CHUNK_SIZE 32
#define CHUNK_SIZE_FLOAT static_cast<GLfloat>(CHUNK_SIZE)

static_assert(CHUNK_SIZE == 32, "Chunk size needs to be 32 due to hardcoded values in greedy meshing");

#include "common.hpp"
#include <vector>
#include "VertContainer.hpp"
#include "Bitmap.hpp"

#include "Vertex.hpp"

// normal {
// 	0 - y (bottom)
// 	1 + y (top)
// 	2 - z (far)
// 	3 + z (near)
// 	4 - x (left)
// 	5 + x (forward)
// }

struct Voxel {
	Bitmap<8> info = 0;
	GLbyte material_id;

	constexpr Voxel() : material_id(-1) {}
	constexpr Voxel(GLbyte material_id) : material_id(material_id) {}
};

struct Chunk {
	// 3D array, [y][z][x] (height, depth, width). this can easily be moved around to test what gets better cache performance
	Voxel voxels[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE];
	bool vertsHaveChanged = true;
	// [i] corresponds to normal == i
	std::vector<Vertex> verts; // I suspect that most chunks will have empty space so I use a vector. idk how bad this is, memory will be extremely sparse. maybe using a fixed size array here will be better, need to test

	// tells what positions are filled by an opaque block or not
	// used as [y][z] to get the bitmask
	// 1 == is filled
	std::array<std::array<Bitmap<CHUNK_SIZE>, CHUNK_SIZE>, CHUNK_SIZE> opaqueMask;

	Voxel &getVoxelAt(const glm::u8vec3 &pos) {
		return voxels[pos.y][pos.z][pos.x];
	}

	constexpr void insertVoxelAt(const glm::u8vec3 &pos, const Voxel &voxel) {
		voxels[pos.y][pos.z][pos.x] = voxel;
		opaqueMask[pos.y][pos.z].setBit(pos.x);
		vertsHaveChanged = true;
	}

	constexpr bool isEmptyAt(const glm::u8vec3 &pos) {
		return ! opaqueMask[pos.y][pos.z][pos.x];
	}

	constexpr bool isEmptyAt(GLubyte x, GLubyte y, GLubyte z) {
		// printf("%d %d %d is %d\n", x, y, z, opaqueMask[y][z][x]);
		return ! opaqueMask[y][z][x];
	}

	constexpr void breakVoxelAt(GLubyte x, GLubyte y, GLubyte z) {
		opaqueMask[y][z].clearBit(x);
		vertsHaveChanged = true;
	}

	constexpr void breakVoxelAt(const glm::u8vec3 &pos) {
		opaqueMask[pos.y][pos.z].clearBit(pos.x);
		vertsHaveChanged = true;
	}

	std::vector<Vertex> getVertx(GLuint normal) {
		if (vertsHaveChanged) {
			rebuildVerts();
		}
		return this->verts;
	}

	// returns how much was added
	constexpr GLuint addVertsTo(VertContainer<Vertex> &_verts) {
		if (vertsHaveChanged) {
			rebuildVerts();
		}
		_verts.add(verts);
		return verts.size();
	}

	constexpr bool voxelAt(GLuint x, GLuint y, GLuint z) {
		return ! isEmptyAt(x, y, z);
	}

	GLbyte get_material(const int axis, const int a, const int b, const int c) {
		if (axis == 0)
			return voxels[c][a][b].material_id;
		else if (axis == 2)
			return voxels[b][c][a].material_id;
		else
			return voxels[a][b][c].material_id;
	}

	void rebuildVerts() {
		vertsHaveChanged = false;

		printf("REBUILD VERTS IS UNFINISHED\n");
		// TODO...............................
	}
};

static_assert(sizeof(Chunk::opaqueMask) == sizeof(uint32_t) * CHUNK_SIZE * CHUNK_SIZE, "ERROR: opaqueMask has unexpected size");

#endif
