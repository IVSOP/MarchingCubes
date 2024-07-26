#ifndef CHUNK_H
#define CHUNK_H
#include <chrono>

#define CHUNK_SIZE 31
#define CHUNK_SIZE_CORNERS (CHUNK_SIZE + 1)

#define CHUNK_SIZE_FLOAT static_cast<GLfloat>(CHUNK_SIZE)
#define CHUNK_SIZE_CORNERS_FLOAT static_cast<GLfloat>(CHUNK_SIZE_CORNERS)

#include <vector>
#include "VertContainer.hpp"
#include "Bitmap.hpp"

#include "Vertex.hpp"

#include "LookupTable.hpp"

#include "Phys.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/norm.hpp> // distance2


#include <tracy/Tracy.hpp>
// TracyOpenGL.hpp

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

	// internaly, it seems like jolt turns triangles into a list of triangles with indices
	// TODO look into optimizing this, maybe calculate the indices myself. it is important that building a chunk is as fast as possible
	// TODO the normals can also be precomputed. for now I'm going doin the easy path
	JPH::TriangleList triangles;
	JPH::Body *body;

	Chunk() {
		// create the physics body
		body = nullptr;
	}

	~Chunk() {
		// call destroyChunk instead?????
		if (body) {
			Phys::destroyBody(body);
		}
	}

	// destructor that can be called manually to clean things up
	// WARNING for now does not reset the vertex/triangles arrays, just clears them (memory still taken up)
	void destroyChunk() {
		if (body) {
			Phys::destroyBody(body);
			body = nullptr;
		}

		// reset bitmask
		// just memset, should be fast
		(void)memset(corners, 0, sizeof(corners));
		// needed??
		(void)memset(materials, 0, sizeof(materials));
		vertsHaveChanged = true;
	}

	// extremely shitty
	constexpr bool isDestroyed() const {
		return (body == nullptr);
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

	Voxel getVoxelAt(const glm::u8vec3 &pos) const {
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

	std::vector<Vertex> &getVerts(GLuint normal) {
		// if (vertsHaveChanged) {
		// 	rebuildVerts();
		// }
		return this->verts;
	}

	// returns how much was added
	constexpr GLuint addVertsTo(VertContainer<Vertex> &_verts) {
		// if (vertsHaveChanged) {
		// 	rebuildVerts();
		// }
		_verts.add(verts);
		return verts.size();
	}

	constexpr GLuint addPointsTo(VertContainer<Point> &_points) {
		// if (vertsHaveChanged) {
		// 	rebuildVerts();
		// }
		_points.add(debug_points);
		return debug_points.size();
	}

	void generateVoxelTriangles(GLuint x, GLuint y, GLuint z) {
		
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

		const glm::vec3 pos_float = glm::vec3(pos);
		glm::vec3 g1, g2, g3;
		JPH::Float3 v1, v2, v3; // TODO fix this mess of conversions

		Vertex vert;
		// for every 3 edges we can make a triangle
		for (int i = 0; i < 16; i += 3) {
			// If edge index is -1, then no further vertices exist in this configuration
			if (edgeIndices[i] == -1) { break; }
			
			// TODO edgeTable[cubedata] already gives all 3 corners (bit [i] == 1 means one of the corners is [i]). test if that approach is faster 

			// triangle for GPU
			const GLint edgeIndexA = edgeIndices[i];
			const GLint edgeIndexB = edgeIndices[i + 1];
			const GLint edgeIndexC = edgeIndices[i + 2];

			// TODO messy conversions
			vert = Vertex(pos, glm::uvec3(edgeIndexA, edgeIndexB, edgeIndexC), materials[y][z][x]);

			verts.push_back(vert);


			// triangle for JPH
			g1 = pos_float + LookupTable::finalCoords[edgeIndexA];
			g2 = pos_float + LookupTable::finalCoords[edgeIndexB];
			g3 = pos_float + LookupTable::finalCoords[edgeIndexC];

			v1.x = g1.x; v1.y = g1.y; v1.z = g1.z;
			v2.x = g2.x; v2.y = g2.y; v2.z = g2.z;
			v3.x = g3.x; v3.y = g3.y; v3.z = g3.z;

			triangles.push_back(JPH::Triangle(v1, v2, v3));
		}
	}

	void rebuildVerts() {
		vertsHaveChanged = false;
		verts.clear();
		debug_points.clear();
		triangles.clear();

		ZoneScoped;

		for (GLuint y = 0; y < CHUNK_SIZE; y ++) {
			for (GLuint z = 0; z < CHUNK_SIZE; z ++) {
				for (GLuint x = 0; x < CHUNK_SIZE; x ++) {
					generateVoxelTriangles(x, y, z);
				}
			}
		}
	}

	// TODO this is bad
	void rebuildBody(const glm::vec3 &coords) {
		if (triangles.size() > 0) {
			if (body != nullptr) {
				Phys::setBodyMeshShape(body, triangles);
			} else {
				body = Phys::createBody(triangles, coords);
			}
		} else { // no triangles
			if (body != nullptr) {
				Phys::destroyBody(body);
				body = nullptr;
			}
		}
	}

	// TODO types and casts are awful here
	// data should already be offset itself
																		// offset that places world on the positive quadrants
	void generate(const glm::ivec3 &chunk_pos, unsigned char *data, int width, const glm::ivec3 &offset, GLbyte material) {
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
					materials[y][z][x] = material;
				}
			}
		}

	}

	// TODO test if doing it like this is faster
	// void addPhysTerrain(JPH::TriangleList &triangles, const glm::ivec3 &offset) const {
	// 	glm::u8vec3 pos, edges;
	// 	glm::vec3 final_pos, g1, g2, g3;
	// 	JPH::Float3 v1, v2, v3; // what a mess

	// 	for (GLuint i = 0; i < verts.size(); i++) {
	// 		pos = verts[i].getLocalPos();
	// 		edges = verts[i].getEdges();

	// 		final_pos = glm::vec3(offset + glm::ivec3(pos));

	// 		g1 = final_pos + LookupTable::finalCoords[edges[0]];
	// 		g2 = final_pos + LookupTable::finalCoords[edges[1]];
	// 		g3 = final_pos + LookupTable::finalCoords[edges[2]];

	// 		v1.x = g1.x; v1.y = g1.y; v1.z = g1.z;
	// 		v2.x = g2.x; v2.y = g2.y; v2.z = g2.z;
	// 		v3.x = g3.x; v3.y = g3.y; v3.z = g3.z;

	// 		triangles.push_back(JPH::Triangle(v1, v2, v3));
	// 	}
	// }

	// TODO EXTREMELY unoptimized
	void breakSphere(const glm::vec3 &center, GLfloat radius_squared, const glm::vec3 &offset) {
		// GLint min_x = glm::clamp(static_cast<GLint>(center.x - radius), 0, CHUNK_SIZE),
		//       max_x = glm::clamp(static_cast<GLint>(center.x + radius), 0, CHUNK_SIZE),
		//       min_y = glm::clamp(static_cast<GLint>(center.y - radius), 0, CHUNK_SIZE),
		//       max_y = glm::clamp(static_cast<GLint>(center.y + radius), 0, CHUNK_SIZE),
		//       min_z = glm::clamp(static_cast<GLint>(center.z - radius), 0, CHUNK_SIZE),
		//       max_z = glm::clamp(static_cast<GLint>(center.z + radius), 0, CHUNK_SIZE);

		for (GLint x = 0; x < CHUNK_SIZE_CORNERS; x++) {
			for (GLint y = 0; y < CHUNK_SIZE_CORNERS; y++) {
				for (GLint z = 0; z < CHUNK_SIZE_CORNERS; z++) {
					if (glm::distance2(glm::vec3(x, y, z) + offset, center) <= radius_squared) {
						// breakVoxelAt(glm::u8vec3(x, y, z));
						breakCornerAt(glm::u8vec3(x, y, z));
					}
				}
			}
		}
	}
};


#endif
