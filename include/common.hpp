#ifndef COMMON_H
#define COMMON_H

#include "stdlib.hpp"
#include "types.hpp"

#include "GLErrors.hpp"

#define WORLD_SIZE_X 16
#define WORLD_SIZE_X_MASK 0x000000FF
#define WORLD_SIZE_Y 4
#define WORLD_SIZE_Y_MASK 0x0000000F
#define WORLD_SIZE_Z 16
#define WORLD_SIZE_Z_MASK 0x000000FF

#define WORLD_SIZE_X_FLOAT static_cast<GLfloat>(WORLD_SIZE_X)
#define WORLD_SIZE_Y_FLOAT static_cast<GLfloat>(WORLD_SIZE_Y)
#define WORLD_SIZE_Z_FLOAT static_cast<GLfloat>(WORLD_SIZE_Z)

#define MAX_X ((CHUNK_SIZE * (WORLD_SIZE_X / 2)) - 1)
#define MAX_Y ((CHUNK_SIZE * (WORLD_SIZE_Y / 2)) - 1)
#define MAX_Z ((CHUNK_SIZE * (WORLD_SIZE_Z / 2)) - 1)

#define MIN_X ((-MAX_X) - 1)
#define MIN_Y ((-MAX_Y) - 1)
#define MIN_Z ((-MAX_Z) - 1)

// normal {
// 	0 - y (bottom)
// 	1 + y (top)
// 	2 - z (far)
// 	3 + z (near)
// 	4 - x (left)
// 	5 + x (right)
// }

#define NORMAL_NEG_Y 0
#define NORMAL_POS_Y 1
#define NORMAL_NEG_Z 2
#define NORMAL_POS_Z 3
#define NORMAL_NEG_X 4
#define NORMAL_POS_X 5

#include "Chunk.hpp"

// had to be here due to include errors
// from the chunk's position in array, return its world position
static constexpr glm::vec3 getChunkCoordsFloat(GLuint x, GLuint y, GLuint z) {
	return
		glm::vec3( // make it so that [half][half][half] is roughly around (0,0,0)
			(static_cast<GLfloat>(x) - WORLD_SIZE_X_FLOAT / 2.0f) * static_cast<GLfloat>(CHUNK_SIZE),
			(static_cast<GLfloat>(y) - WORLD_SIZE_Y_FLOAT / 2.0f) * static_cast<GLfloat>(CHUNK_SIZE),
			(static_cast<GLfloat>(z) - WORLD_SIZE_Z_FLOAT / 2.0f) * static_cast<GLfloat>(CHUNK_SIZE)
		);
}

static constexpr glm::ivec3 getChunkCoords(GLuint x, GLuint y, GLuint z) {
	return
		glm::ivec3( // make it so that [half][half][half] is roughly around (0,0,0)
			((static_cast<GLint>(x) - WORLD_SIZE_X / 2) * CHUNK_SIZE),
			((static_cast<GLint>(y) - WORLD_SIZE_Y / 2) * CHUNK_SIZE),
			((static_cast<GLint>(z) - WORLD_SIZE_Z / 2) * CHUNK_SIZE)
		);
}

// same as other ones but uses chunk ID
// DOES NOT WORK!!!!!!!!!!!!!!!!
static constexpr glm::ivec3 getChunkCoordsByID(GLuint ID) {
	// what the fuck?
	GLuint x = ID / (WORLD_SIZE_X * WORLD_SIZE_Y);
	GLuint idk = ID -  x * WORLD_SIZE_X * WORLD_SIZE_Y;
	GLuint y = idk / WORLD_SIZE_X;
	GLuint z = idk % WORLD_SIZE_X;

	// xyz are now [x][y][z] where chunk is located. this means this is not very optimized but since everytthing is constexpr I trust the compiler will manage this
	return getChunkCoords(x, y, z);
}

#define PHYS_FPS 60.0f
#define PHYS_STEP 1.0f / PHYS_FPS

const GLchar *readFromFile(const char *filepath);

// idk where else to put this
struct SelectedBlockInfo {
	GLint materialID;
	GLuint chunkID;
	GLubyte normal;
	bool _isEmpty;
	glm::u8vec3 local_pos; // relative to chunk
	glm::ivec3 world_pos;

	SelectedBlockInfo(GLint materialID, GLuint chunkID, GLubyte normal, bool isEmpty, const glm::u8vec3 &local_pos, const glm::ivec3 &world_pos)
	: materialID(materialID), chunkID(chunkID), normal(normal), _isEmpty(isEmpty), local_pos(local_pos), world_pos(world_pos) {}

	SelectedBlockInfo() = default;

	constexpr bool isEmpty() const { return _isEmpty; }

	glm::ivec3 getWorldPosition() const {
		// return glm::vec3(getChunkCoordsByID(chunkID)) + glm::vec3(position);
		return world_pos;
	}
};

// same
struct IndirectData {
	// for use with glMultiDrawArraysIndirect!!!!!!!!!!!
	GLuint count;		  // Number of vertices per instance. I set it to 3 since I am instancing 4B of information as 3 vertices
	GLuint instanceCount; // Number of instances to draw, num of triangles in a chunk
	GLuint first;		  // Offset into the array of vertices (???????????)
	GLuint baseInstance;  // instance offset

// same as glDrawArraysInstancedBaseInstance(mode,     cmd->first,             cmd->count,         cmd->instanceCount,             cmd->baseInstance); many times


	IndirectData()
	: count(3), first(0) {}

	IndirectData(GLuint baseInstance, GLuint instanceCount)
	: count(3), instanceCount(instanceCount), first(0), baseInstance(baseInstance) {}

	~IndirectData() = default;
};

// wrapper around a simple pointer. did not want to include the size into the template for now
template<typename T> struct custom_array {
	const T* _data;
	const GLsizei _size;

	constexpr GLsizei size() const { return _size; }
	constexpr const T* data() const { return _data; }

	custom_array(T* data, GLsizei size)
	: _data(data), _size(size) {}
	~custom_array() = default;
};

enum Camera_Movement {
    FORWARD,
	FRONT, // paralel to the ground
    BACKWARD,
	BACK, // paralel to the ground
    LEFT, // paralel to the ground (they are like this by default)
    RIGHT, // paralel to the ground (they are like this by default)
	UP, // paralel to the ground
	DOWN // paralel to the ground
};

static constexpr std::size_t getNextPowerOfTwo(std::size_t num) {
	num--;
	num |= num >> 1;
	num |= num >> 2;
	num |= num >> 4;
	num |= num >> 8;
	num |= num >> 16;
	num |= num >> 32;
	num++;
	return num;
}


#endif
