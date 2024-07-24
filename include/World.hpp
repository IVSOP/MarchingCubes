#ifndef WORLD_H
#define WORLD_H

#include "common.hpp"
#include "Chunk.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/norm.hpp> // distance2

#include <entt.hpp>
#include "Phys.hpp"

// #include "zlib/zlib.h"

struct World {
	Chunk chunks[WORLD_SIZE_X][WORLD_SIZE_Y][WORLD_SIZE_Z]; // this order can be changed, need to test it for performance

	VertContainer<Vertex> verts; // so I dont have to constantly alloc and free
	VertContainer<Point> debug_points;
	std::vector<IndirectData> indirect;
	std::vector<ChunkInfo> info;
	entt::registry entt_registry;


	constexpr Chunk &get(const glm::uvec3 &position) {
		return chunks[position.x][position.y][position.z];
	}

	VertContainer<Vertex> &getVerts() {
		return verts;
	}

	VertContainer<Point> &getPoints() {
		return debug_points;
	}

	std::vector<IndirectData> &getIndirect() {
		return indirect;
	}

	std::vector<ChunkInfo> &getInfo() {
		return info;
	}

	void buildData();

	constexpr GLuint getChunkID(GLuint x, GLuint y, GLuint z) {
		return (&chunks[x][y][z] - &chunks[0][0][0]);
	}

	// from position within chunk, retrieve position in the world
	constexpr glm::ivec3 getWorldCoords(GLuint chunkID, glm::u8vec3 position) { // const
		const glm::ivec3 chunk_offset = getChunkCoordsByID(chunkID);
		return chunk_offset + glm::ivec3(position);
	}

	World()
	: verts(1 << 10), debug_points(1 << 10), indirect(1 << 10), info(1 << 10) // why tf is 2e10 == 20000000000?????????????
	{
		// the default constructor of each chunk creates an empty body
		// or it would if that was possible
	}

	void copyChunkTo(const Chunk &chunk, const glm::uvec3 position) {
		chunks[position.x][position.y][position.z] = chunk;
		chunks[position.x][position.y][position.z].vertsHaveChanged = true;
	}

	SelectedBlockInfo getBlockInfo(const glm::ivec3 &position);

	constexpr float custom_mod (float value, float modulus) {
		return fmod(fmod(value, modulus) + modulus, modulus);
	}

	constexpr float intbound(float s, float ds) {
		// Find the smallest positive t such that s+t*ds is an integer.
		if (ds < 0) {
			return intbound(-s, -ds);
		} else {
			s = custom_mod(s, 1.0f);
			// The problem is now s + t * ds = 1.0
			return (1.0f - s) / ds;
		}
	}

	constexpr int signum(float x) {
		return (x > 0) ? 1 : (x < 0) ? -1 : 0;
	}

	SelectedBlockInfo getSelectedBlock(const glm::vec3 &position, const glm::vec3 &lookPosition, GLfloat radius);

	// this being a reference is VERY cursed!!!!!!!
	Chunk &getChunkByID(GLuint chunkID) {
		Chunk * cursed_ptr = reinterpret_cast<Chunk *>(chunks);
		return cursed_ptr[chunkID];
	}

	// // SelectedBlockInfo is what the caller will have, and it contains all the information needed to do this
	// void breakVoxel(const SelectedBlockInfo &selectedInfo) {
	// 	Chunk &chunk = getChunkByID(selectedInfo.chunkID);
	// 	chunk.breakVoxelAt(selectedInfo.position);
	// }

	// void breakVoxel(const glm::ivec3 position) {
	// 	// got lazy, this will automatically get chunk and position inside it
	// 	const SelectedBlockInfo blockInfo = getBlockInfo(position);

	// 	Chunk &chunk = getChunkByID(blockInfo.chunkID);
	// 	chunk.breakVoxelAt(blockInfo.position);
	// }

	// void setVoxelValue(const glm::ivec3 position, const Bitmap<8> &value) {
	// 	// got lazy, this will automatically get chunk and position inside it
	// 	const SelectedBlockInfo blockInfo = getBlockInfo(position);

	// 	Chunk &chunk = getChunkByID(blockInfo.chunkID);
	// 	chunk.setVoxelValue(blockInfo.position, value);
	// }

	// void maskVoxelValue(const glm::ivec3 position, const Bitmap<8> &mask) {
	// 	// got lazy, this will automatically get chunk and position inside it
	// 	const SelectedBlockInfo blockInfo = getBlockInfo(position);

	// 	Chunk &chunk = getChunkByID(blockInfo.chunkID);
	// 	chunk.maskVoxelValue(blockInfo.position, mask);
	// }

	// TODO this can be optimized like crazy
	// some ideas:
	// checking if something in the big BB is inside the small BB is done in the shitiest way possible, using isDestroyed()
	// start considering chunk center instead of the corner, as its position
	// move shpere to center of nearest chunk. 1 unit moved = 1 unit reduction in radius (this is usefull for the smaller sphere etc)
	// clamping takes min and max, sometimes only one is needed
	void breakVoxelSphere(const SelectedBlockInfo &selectedInfo, GLfloat radius);

	void loadHeightMap(const std::string &path);
};


#endif
