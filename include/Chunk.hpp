#ifndef CHUNK_H
#define CHUNK_H

#include <chrono>

#define CHUNK_SIZE 32
#define CHUNK_SIZE_FLOAT static_cast<GLfloat>(CHUNK_SIZE)

static_assert(CHUNK_SIZE == 32, "Chunk size needs to be 32 due to hardcoded values in greedy meshing");

#include <vector>
#include "VertContainer.hpp"
#include "Bitmap.hpp"

#include "Vertex.hpp"

#include "LookupTable.hpp"

// normal {
// 	0 - y (bottom)
// 	1 + y (top)
// 	2 - z (far)
// 	3 + z (near)
// 	4 - x (left)
// 	5 + x (forward)
// }

struct Voxel {
	Bitmap<8> data = 0;
	GLbyte material_id;

	constexpr Voxel() : data(0), material_id(-1) {}
	constexpr Voxel(uint8_t data, GLbyte material_id) : data(data), material_id(material_id) {}
};

struct ChunkInfo {
	glm::vec3 position;
	GLfloat padding = 0.0f;
	
	ChunkInfo() : position(0.0f) {}
	ChunkInfo(const glm::vec3 &position) : position(position) {};
};

struct Chunk {
	// 3D array, [y][z][x] (height, depth, width). this can easily be moved around to test what gets better cache performance
	Voxel voxels[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE];
	bool vertsHaveChanged = true;
	// [i] corresponds to normal == i
	std::vector<Vertex> verts; // I suspect that most chunks will have empty space so I use a vector. idk how bad this is, memory will be extremely sparse. maybe using a fixed size array here will be better, need to test
	std::vector<Point> debug_points;

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

	std::vector<Vertex> getVerts(GLuint normal) {
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

	constexpr GLuint addPointsTo(VertContainer<Point> &_points) {
		if (vertsHaveChanged) {
			rebuildVerts();
		}
		_points.add(debug_points);
		return debug_points.size();
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

	void generateVoxelTriangles(std::vector<Vertex> &verts, std::vector<Point> &points, GLuint x, GLuint y, GLuint z) const {
		// Bitmap<8> cubedata = voxels[y][z][x].data;
		uint8_t cubedata = voxels[y][z][x].data.data; // cursed but whatever, will change in the future

		// if it is completely inside or outside, nothing gets drawn
		// idk if this makes anything faster, but it helps when using debug points
		if (cubedata == 0xFF || cubedata == 0x00) {
			return;
		}

		const glm::vec3 pos_in_chunk = glm::vec3(static_cast<GLfloat>(x), static_cast<GLfloat>(y), static_cast<GLfloat>(z));
		for (GLubyte corner = 0; corner < 8; corner ++) {
			if (cubedata & (1 << corner)) {
				points.emplace_back(LookupTable::corner_coords[corner] + pos_in_chunk);
			}
		}

		// for this configuration, get list of indices corresponding to 'activated' edges
		const int8_t *edgeIndices = LookupTable::triTable[cubedata];

		Vertex vert;
		// for every 3 edges we can make a triangle
		for (int i = 0; i < 16; i += 3) {
			// If edge index is -1, then no further vertices exist in this configuration
			if (edgeIndices[i] == -1) { break; }
			
			// TODO edgeTable[cubedata] already gives all 3 corners (bit [i] == 1 means one of the corners is [i]). test if that approach is faster 

			// instead of doing the commented approach, since I always just get the mid point instead of interpolating,
			// this means I can precompute the values to be faster
				// // for the 3 current edges being considered, get the two corners that define them
				// int edgeIndexA = edgeIndices[i];
				// int a0 = LookupTable::cornerIndexAFromEdge[edgeIndexA];
				// int a1 = LookupTable::cornerIndexBFromEdge[edgeIndexA];

				// int edgeIndexB = edgeIndices[i + 1];
				// int b0 = LookupTable::cornerIndexAFromEdge[edgeIndexB];
				// int b1 = LookupTable::cornerIndexBFromEdge[edgeIndexB];

				// int edgeIndexC = edgeIndices[i + 2];
				// int c0 = LookupTable::cornerIndexAFromEdge[edgeIndexC];
				// int c1 = LookupTable::cornerIndexBFromEdge[edgeIndexC];

				// // Calculate positions of each vertex
				// // instead of interpolating, I get the mid point
				// triangle[0].coords = ((LookupTable::corner_coords[a0] + LookupTable::corner_coords[a1]) / 2.0f) + pos_in_chunk;
				// triangle[1].coords = ((LookupTable::corner_coords[b0] + LookupTable::corner_coords[b1]) / 2.0f) + pos_in_chunk;
				// triangle[2].coords = ((LookupTable::corner_coords[c0] + LookupTable::corner_coords[c1]) / 2.0f) + pos_in_chunk;

			const GLint edgeIndexA = edgeIndices[i];
			const GLint edgeIndexB = edgeIndices[i + 1];
			const GLint edgeIndexC = edgeIndices[i + 2];

			// TODO messy conversions
			vert = Vertex(glm::uvec3(x, y, z), glm::uvec3(edgeIndexA, edgeIndexB, edgeIndexC), 0);

			verts.push_back(vert);
		}
	}

	void rebuildVerts() {
		vertsHaveChanged = false;
		verts.clear();
		debug_points.clear();

		for (GLuint y = 0; y < CHUNK_SIZE; y ++) {
			for (GLuint z = 0; z < CHUNK_SIZE; z ++) {
				for (GLuint x = 0; x < CHUNK_SIZE; x ++) {
					generateVoxelTriangles(verts, debug_points, x, y, z);
				}
			}
		}
	}

	// TODO types and casts are awful here
	// data should already be offset itself
																		// offset that places world on the positive quadrants
	void generate(const glm::ivec3 &chunk_pos, unsigned char *data, int width, const glm::ivec3 &offset) {
		GLubyte value = 0;
		glm::ivec3 pos;
		unsigned char height;

		// change to int???
		for (GLuint y = 0; y < CHUNK_SIZE; y++) {
			for (GLuint z = 0; z < CHUNK_SIZE; z++) {
				for (GLuint x = 0; x < CHUNK_SIZE; x++) {

					pos = chunk_pos + glm::ivec3(x, y, z);

					for (GLubyte corner = 0; corner < 8; corner++) {


														// avoid this conversion, precompute this
						const glm::ivec3 final_position = glm::ivec3(LookupTable::corner_coords[corner]) + pos + offset;
						
						height = data[final_position.z * width + final_position.x];

						if (final_position.y <= static_cast<GLint>(height)) {
							value |= 1 << corner;
						}
					}

					// not needed??
					if (value != 0x00) {
						// could do it faster
						insertVoxelAt(glm::uvec3(x, y, z), Voxel(value, 0));
					}

					value = 0;
				}
			}
		}
	}
};

static_assert(sizeof(Chunk::opaqueMask) == sizeof(uint32_t) * CHUNK_SIZE * CHUNK_SIZE, "ERROR: opaqueMask has unexpected size");

#endif
