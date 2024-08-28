#include "Chunk.hpp"
#include "Profiling.hpp"

void Chunk::generateVoxelTriangles(GLuint x, GLuint y, GLuint z) {
	
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


	const glm::vec3 pos_float = glm::vec3(pos);
	glm::vec3 g1, g2, g3;
	JPH::Float3 v1, v2, v3; // TODO fix this mess of conversions
	// glm::vec3 normal;

#ifdef OLD_MARCHING_CUBES
	// for this configuration, get list of indices corresponding to 'activated' edges
	const int8_t *edgeIndices = LookupTable::triTable[cubedata];


	// for every 3 edges we can make a triangle
	for (int i = 0; i < 16; i += 3) {
		// If edge index is -1, then no further vertices exist in this configuration
		if (edgeIndices[i] == -1) { break; }
		
		// TODO edgeTable[cubedata] already gives all 3 corners (bit [i] == 1 means one of the corners is [i]). test if that approach is faster 
		const GLint edgeIndexA = edgeIndices[i];
		const GLint edgeIndexB = edgeIndices[i + 1];
		const GLint edgeIndexC = edgeIndices[i + 2];

#else
	
	const uint8_t num_triangles = LookupTable::num_edges[cubedata];
	const uint8_t *edges = LookupTable::edges[cubedata];

	for (unsigned int i = 0; i < num_triangles; i ++) {
		
		// triangle for GPU
		const GLint edgeIndexA = edges[i * 3];
		const GLint edgeIndexB = edges[(i * 3) + 1];
		const GLint edgeIndexC = edges[(i * 3) + 2];

#endif

		// TODO messy conversions
		verts.emplace_back(pos, glm::u8vec3(edgeIndexA, edgeIndexB, edgeIndexC), materials[y][z][x]);


		// triangle for JPH
		g1 = pos_float + LookupTable::finalCoords[edgeIndexA];
		g2 = pos_float + LookupTable::finalCoords[edgeIndexB];
		g3 = pos_float + LookupTable::finalCoords[edgeIndexC];

		v1.x = g1.x; v1.y = g1.y; v1.z = g1.z;
		v2.x = g2.x; v2.y = g2.y; v2.z = g2.z;
		v3.x = g3.x; v3.y = g3.y; v3.z = g3.z;

		triangles.push_back(JPH::Triangle(v1, v2, v3));

		// normal = LookupTable::normals[edgeIndexA][edgeIndexB][edgeIndexC];
		// normals.push_back(JPH::Vec3(normal.x, normal.y, normal.z));
	}
}

void Chunk::rebuildVerts() {
	vertsHaveChanged = false;
	verts.clear();
	// debug_points.clear();
	triangles.clear();
	// normals.clear();

	ZoneScoped;

	for (GLuint y = 0; y < CHUNK_SIZE; y ++) {
		for (GLuint z = 0; z < CHUNK_SIZE; z ++) {
			for (GLuint x = 0; x < CHUNK_SIZE; x ++) {
				generateVoxelTriangles(x, y, z);
			}
		}
	}
}

// TODO types and casts are awful here
// data should already be offset itself
																	// offset that places world on the positive quadrants
void Chunk::generate(const glm::ivec3 &chunk_pos, const unsigned char *data, int width, const glm::ivec3 &offset, GLbyte material) {
	destroyed = false;
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

// TODO EXTREMELY unoptimized
void Chunk::breakSphere(const glm::vec3 &center, GLfloat radius_squared, const glm::vec3 &offset) {
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

void Chunk::addSphere(const glm::vec3 &center, GLfloat radius_squared, const glm::vec3 &offset) {
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
					addCornerAt(glm::u8vec3(x, y, z), 1);
				}
			}
		}
	}
}
