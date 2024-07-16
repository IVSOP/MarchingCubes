#ifndef CHUNK_H
#define CHUNK_H
#include <chrono>

#define CHUNK_SIZE 31
#define CHUNK_SIZE_CORNERS (CHUNK_SIZE + 1)

#define CHUNK_SIZE_FLOAT static_cast<GLfloat>(CHUNK_SIZE)

#include <vector>
#include "VertContainer.hpp"
#include "Bitmap.hpp"

#include "Vertex.hpp"

#include "LookupTable.hpp"

#include <Jolt/Jolt.h>
#include <Jolt/RegisterTypes.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/CylinderShape.h>
#include <Jolt/Physics/Collision/Shape/RotatedTranslatedShape.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/MeshShape.h>


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
	Bitmap<32> corners[CHUNK_SIZE_CORNERS][CHUNK_SIZE_CORNERS];
	GLbyte materials[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE];

	bool vertsHaveChanged = true;
	// [i] corresponds to normal == i
	std::vector<Vertex> verts; // I suspect that most chunks will have empty space so I use a vector. idk how bad this is, memory will be extremely sparse. maybe using a fixed size array here will be better, need to test
	std::vector<Point> debug_points;

	Bitmap<8> getDataAt(const glm::u8vec3 &pos) const {
		Bitmap<8> data = 0; // = 0 not needed???

		// TODO!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! this is what this should look like:
		// data = static_cast<uint8_t>( (corners[pos.y][pos.z].data >> pos.x)         & 0x00000003)       |
		// 	   static_cast<uint8_t>(((corners[pos.y][pos.z + 1].data >> pos.x)     & 0x00000003) << 2) |
		// 	   static_cast<uint8_t>(((corners[pos.y + 1][pos.z].data >> pos.x)     & 0x00000003) << 4) |
		// 	   static_cast<uint8_t>(((corners[pos.y + 1][pos.z + 1].data >> pos.x) & 0x00000003) << 6);

		// but the place where I got my marching cubes data has a weird order for the vertices of the cube
		// for now this is fast enough but in the future I need to change it
		data = static_cast<uint8_t>( (corners[pos.y][pos.z].data >> pos.x)               & 0x00000003)       | // 0,1
			   static_cast<uint8_t>(((corners[pos.y][pos.z + 1].data >> (pos.x + 1))     & 0x00000001) << 2) | // 2
			   static_cast<uint8_t>(((corners[pos.y][pos.z + 1].data >> pos.x)           & 0x00000001) << 3) | // 3
			   static_cast<uint8_t>(((corners[pos.y + 1][pos.z].data >> pos.x)           & 0x00000003) << 4) | // 4, 5
			   static_cast<uint8_t>(((corners[pos.y + 1][pos.z + 1].data >> (pos.x + 1)) & 0x00000001) << 6) | // 6
			   static_cast<uint8_t>(((corners[pos.y + 1][pos.z + 1].data >> pos.x)       & 0x00000001) << 7);  // 7

		return data;

	}

	Voxel getVoxelAt(const glm::u8vec3 &pos) const {
		return Voxel(getDataAt(pos).data, materials[pos.y][pos.z][pos.x]);
	}

	// constexpr void insertVoxelAt(const glm::u8vec3 &pos, const Voxel &voxel) {
	// 	voxels[pos.y][pos.z][pos.x] = voxel;
	// 	vertsHaveChanged = true;
	// }

	// constexpr bool isEmptyAt(const glm::u8vec3 &pos) {
	// 	return (voxels[pos.y][pos.z][pos.x].data.allFalse());
	// }

	// constexpr void breakVoxelAt(GLubyte x, GLubyte y, GLubyte z) {
	// 	vertsHaveChanged = true;
	// }

	// constexpr void breakVoxelAt(const glm::u8vec3 &pos) {
	// 	voxels[pos.y][pos.z][pos.x].data.clear();
	// 	vertsHaveChanged = true;
	// }

	// constexpr void setVoxelValue(const glm::u8vec3 &pos, const Bitmap<8> &value) {
	// 	voxels[pos.y][pos.z][pos.x].data = value;
	// 	vertsHaveChanged = true;
	// }

	// constexpr void maskVoxelValue(const glm::u8vec3 &pos, const Bitmap<8> &mask) {
	// 	voxels[pos.y][pos.z][pos.x].data &= mask;
	// 	vertsHaveChanged = true;
	// }

	// constexpr bool voxelAt(GLuint x, GLuint y, GLuint z) {
	// 	return ! isEmptyAt(x, y, z);
	// }

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

	void generateVoxelTriangles(std::vector<Vertex> &verts, std::vector<Point> &points, GLuint x, GLuint y, GLuint z) const {
		
		// for (GLuint y = 0; y < CHUNK_SIZE; y++) {
		// 	for (GLuint z = 0; z < CHUNK_SIZE; z++) {
		// 		if (corners[y][z].data != 0xFFFFFFFF && corners[y][z].data != 0x00000000) {
		// 			corners[y][z].print();
		// 		}
		// 	}
		// }

		// Bitmap<8> cubedata = voxels[y][z][x].data;
		const glm::u8vec3 pos = glm::u8vec3(x, y, z);
		uint8_t cubedata = getDataAt(pos).data;


		// if it is completely inside or outside, nothing gets drawn
		// idk if this makes anything faster, but it helps when using debug points
		if (cubedata == 0xFF || cubedata == 0x00) {
			return;
		}

		// const glm::vec3 pos_in_chunk = glm::vec3(static_cast<GLfloat>(x), static_cast<GLfloat>(y), static_cast<GLfloat>(z));
		// for (GLubyte corner = 0; corner < 8; corner ++) {
		// 	if (cubedata & (1 << corner)) {
		// 		points.emplace_back(LookupTable::corner_coords[corner] + pos_in_chunk);
		// 	}
		// }

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
		glm::ivec3 pos;
		unsigned char height;

		// change to int???
		for (GLuint y = 0; y < CHUNK_SIZE_CORNERS; y++) {
			for (GLuint z = 0; z < CHUNK_SIZE_CORNERS; z++) {
				corners[y][z].clear();
				for (GLuint x = 0; x < CHUNK_SIZE_CORNERS; x++) {

					pos = chunk_pos + glm::ivec3(x, y, z) + offset; // pos of the corner + offset

					height = data[pos.z * width + pos.x];
					// if height of voxel <= height of heightmap
					if (pos.y <= static_cast<GLint>(height)) {
						corners[y][z].setBit(x);
					}
				}
			}
		}

		for (GLuint y = 0; y < CHUNK_SIZE; y++) {
			for (GLuint z = 0; z < CHUNK_SIZE; z++) {
				for (GLuint x = 0; x < CHUNK_SIZE; x++) {
					materials[y][z][x] = 0;
				}
			}
		}

	}

	void addPhysTerrain(JPH::TriangleList &triangles, const glm::ivec3 &offset) const {
		glm::u8vec3 pos, edges;
		glm::vec3 final_pos, g1, g2, g3;
		JPH::Float3 v1, v2, v3; // what a mess

		for (GLuint i = 0; i < verts.size(); i++) {
			pos = verts[i].getLocalPos();
			edges = verts[i].getEdges();

			final_pos = glm::vec3(offset + glm::ivec3(pos));

			g1 = final_pos + LookupTable::finalCoords[edges[0]];
			g2 = final_pos + LookupTable::finalCoords[edges[1]];
			g3 = final_pos + LookupTable::finalCoords[edges[2]];

			v1.x = g1.x; v1.y = g1.y; v1.z = g1.z;
			v2.x = g2.x; v2.y = g2.y; v2.z = g2.z;
			v3.x = g3.x; v3.y = g3.y; v3.z = g3.z;

			triangles.push_back(JPH::Triangle(v1, v2, v3));
		}
	}
};


#endif
