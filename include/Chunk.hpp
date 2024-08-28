#ifndef CHUNK_H
#define CHUNK_H
#include <chrono>

// num cubes
#define CHUNK_SIZE 31
// num corners
#define CHUNK_SIZE_CORNERS (CHUNK_SIZE + 1)

#define CHUNK_SIZE_FLOAT static_cast<GLfloat>(CHUNK_SIZE)
#define CHUNK_SIZE_CORNERS_FLOAT static_cast<GLfloat>(CHUNK_SIZE_CORNERS)

#include <vector>
#include "Bitmap.hpp"

#include "Vertex.hpp"

#include "LookupTable.hpp"

#include "Phys.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/norm.hpp> // distance2

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
	// TODO test this performance vs a vector
	CustomVec<Vertex> verts; // I suspect that most chunks will have empty space so I use a vector. idk how bad this is, memory will be extremely sparse. maybe using a fixed size array here will be better, need to test
	// std::vector<Point> debug_points;

	// internaly, it seems like jolt turns triangles into a list of triangles with indices
	// TODO look into optimizing this, maybe calculate the indices myself. it is important that building a chunk is as fast as possible
	// TODO the normals can also be precomputed. for now I'm going doin the easy path
	JPH::TriangleList triangles;
	// std::vector<JPH::Vec3> normals;
	JPH::Body *body;
	bool destroyed; // TODO I don't like this

	Chunk()
	: verts(1)
	{
		// create the physics body
		body = nullptr;
		destroyed = true;

		(void)memset(corners, 0, sizeof(corners));
		(void)memset(materials, 0, sizeof(materials));
	}

	~Chunk() {
		// call destroyChunk instead?????
		if (body) {
			Phys::destroyBody(body);
		}
	}


	// destroyChunk and reviveChunk are pretty cursed but needed due to the way I manage chunks, they are slightly faster than destroying them

	// destructor that can be called manually to clean things up
	// WARNING will clear the std::vectors and their memory
	void destroyChunk() {
		destroyed = true;

		if (body) {
			Phys::destroyBody(body);
			body = nullptr; // for safety, I would rather get a segfault than weird bugs
		}

		// verts.reset();
		// verts.clear();
		// verts.shrink_to_fit();
		verts.reset();
		triangles.clear();
		triangles.shrink_to_fit();

		// reset bitmask
		// just memset, should be fast
		(void)memset(corners, 0, sizeof(corners));
		// needed??
		(void)memset(materials, 0, sizeof(materials));
		vertsHaveChanged = true;
	}

	void reviveChunk(const glm::vec3 &coords) {
		destroyed = false;
		vertsHaveChanged = false;
		rebuildBody(coords);
	}

	// extremely shitty
	constexpr bool isDestroyed() const {
		return destroyed;
	}

	constexpr Bitmap<8> getDataAt(const glm::u8vec3 &pos) const {
		Bitmap<8> data = 0; // = 0 not needed???

		// TODO!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! this is what this should look like:
		// data = static_cast<uint8_t>( (corners[pos.y][pos.z].data >> pos.x)         & 0x00000003)       |
		// 	   static_cast<uint8_t>(((corners[pos.y][pos.z + 1].data >> pos.x)     & 0x00000003) << 2) |
		// 	   static_cast<uint8_t>(((corners[pos.y + 1][pos.z].data >> pos.x)     & 0x00000003) << 4) |
		// 	   static_cast<uint8_t>(((corners[pos.y + 1][pos.z + 1].data >> pos.x) & 0x00000003) << 6);

		// but the place where I got my marching cubes data has a weird order for the vertices of the cube
		// for now this is fast enough but in the future I need to change it
		data = static_cast<uint8_t>( (corners[pos.y][pos.z].data         >> pos.x)       & 0x00000003)       | // 0,1
			   static_cast<uint8_t>(((corners[pos.y][pos.z + 1].data     >> (pos.x + 1)) & 0x00000001) << 2) | // 2
			   static_cast<uint8_t>(((corners[pos.y][pos.z + 1].data     >> pos.x)       & 0x00000001) << 3) | // 3
			   static_cast<uint8_t>(((corners[pos.y + 1][pos.z].data     >> pos.x)       & 0x00000003) << 4) | // 4, 5
			   static_cast<uint8_t>(((corners[pos.y + 1][pos.z + 1].data >> (pos.x + 1)) & 0x00000001) << 6) | // 6
			   static_cast<uint8_t>(((corners[pos.y + 1][pos.z + 1].data >> pos.x)       & 0x00000001) << 7);  // 7

		return data;

	}

	constexpr void setDataAt(const glm::u8vec3 &pos, const Bitmap<8> _data) {

		const uint32_t data = static_cast<uint32_t>(_data.data);

		// cursed wtf
		corners[pos.y][pos.z].data         &= ~(0x00000003 << pos.x);
		corners[pos.y][pos.z + 1].data     &= ~(0x00000001 << (pos.x + 1));
		corners[pos.y][pos.z + 1].data     &= ~(0x00000001 << pos.x);
		corners[pos.y + 1][pos.z].data     &= ~(0x00000003 << pos.x);
		corners[pos.y + 1][pos.z + 1].data &= ~(0x00000001 << (pos.x + 1));
		corners[pos.y + 1][pos.z + 1].data &= ~(0x00000001 << pos.x);

		corners[pos.y][pos.z].data         |= ((data & 0x00000003) << pos.x);
		corners[pos.y][pos.z + 1].data     |= (((data >> 2) & 0x00000001) << (pos.x + 1));
		corners[pos.y][pos.z + 1].data     |= (((data >> 3) & 0x00000001) << pos.x);
		corners[pos.y + 1][pos.z].data     |= (((data >> 4) & 0x00000003) << pos.x);
		corners[pos.y + 1][pos.z + 1].data |= (((data >> 6) & 0x00000001) << (pos.x + 1));
		corners[pos.y + 1][pos.z + 1].data |= (((data >> 7) & 0x00000001) << pos.x);
	}

	constexpr Voxel getVoxelAt(const glm::u8vec3 &pos) const {
		return Voxel(getDataAt(pos).data, materials[pos.y][pos.z][pos.x]);
	}

	// constexpr void insertVoxelAt(const glm::u8vec3 &pos, const Voxel &voxel) {
	// 	voxels[pos.y][pos.z][pos.x] = voxel;
	// 	vertsHaveChanged = true;
	// }

	// specific for a voxel, check is all bits around this position are empty
	constexpr bool isVoxelEmptyAt(const glm::u8vec3 &pos) {
		return (getDataAt(pos).allFalse());
	}

	// constexpr void breakVoxelAt(GLubyte x, GLubyte y, GLubyte z) {
	// 	vertsHaveChanged = true;
	// }

	constexpr void breakVoxelAt(const glm::u8vec3 &pos) {
		// constexpr Bitmap<8> data = 0x00;
		// setDataAt(pos, data);

		corners[pos.y][pos.z].data         &= ~(0x00000003 << pos.x);
		corners[pos.y][pos.z + 1].data     &= ~(0x00000001 << (pos.x + 1));
		corners[pos.y][pos.z + 1].data     &= ~(0x00000001 << pos.x);
		corners[pos.y + 1][pos.z].data     &= ~(0x00000003 << pos.x);
		corners[pos.y + 1][pos.z + 1].data &= ~(0x00000001 << (pos.x + 1));
		corners[pos.y + 1][pos.z + 1].data &= ~(0x00000001 << pos.x);

		vertsHaveChanged = true; // do not do this here since this gets called many times TODO
	}

	constexpr void breakCornerAt(const glm::u8vec3 &pos) {
		corners[pos.y][pos.z].clearBit(pos.x);

		vertsHaveChanged = true; // do not do this here since this gets called many times TODO
	}

	constexpr void addCornerAt(const glm::u8vec3 &pos, GLbyte material_id) {
		corners[pos.y][pos.z].setBit(pos.x);

		// TODO set material, CAREFULL this pos is < 32, materials are <31

		vertsHaveChanged = true; // do not do this here since this gets called many times TODO
	}

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

	constexpr CustomVec<Vertex> &getVerts(GLuint normal) {
		// if (vertsHaveChanged) {
		// 	rebuildVerts();
		// }
		return this->verts;
	}

	// returns how much was added
	constexpr GLuint addVertsTo(CustomVec<Vertex> &_verts) {
		// if (vertsHaveChanged) {
		// 	rebuildVerts();
		// }
		_verts.add(verts);
		return verts.size();
	}

	constexpr GLuint addPointsTo(CustomVec<Point> &_points) {
		// if (vertsHaveChanged) {
		// 	rebuildVerts();
		// }
		// _points.add(debug_points);
		// return debug_points.size();
		return 0;
	}

	void generateVoxelTriangles(GLuint x, GLuint y, GLuint z);

	void rebuildVerts();

	// TODO types and casts are awful here
	// data should already be offset itself
																		// offset that places world on the positive quadrants
	void generate(const glm::ivec3 &chunk_pos, const unsigned char *data, int width, const glm::ivec3 &offset, GLbyte material);

	// TODO EXTREMELY unoptimized
	void breakSphere(const glm::vec3 &center, GLfloat radius_squared, const glm::vec3 &offset);
	void addSphere(const glm::vec3 &center, GLfloat radius_squared, const glm::vec3 &offset);

	// TODO this is bad
	void rebuildBody(const glm::vec3 &coords) {
		if (triangles.size() > 0) {
			if (body != nullptr) {
				Phys::setBodyMeshShape(body, triangles);
			} else {
				// body = Phys::createBodyWithNormals(triangles, coords, normals.data());
				body = Phys::createTerrain(triangles, coords);
			}
		} else { // no triangles
			if (body != nullptr) {
				Phys::destroyBody(body);
				body = nullptr;
			}
		}
	}
};


#endif
