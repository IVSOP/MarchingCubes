#include "World.hpp"

// #define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// #define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize2.h"

#include "Logs.hpp"
#include "Crash.hpp"

/*
DESIGN CHOICES:

I concluded that having the chunks be dumb and the world managing them was the best way to do things
Before, doing addVertsTo() would rebuild the vertices if necessary
But, now that I have physics and need to update the triangles that make up the body's mesh,
I needed to have the chunk itself set its own triangles to its own body, which feels ver messy
So I shifted control into here, and here we check if the triangles need to be rebuilt or not and do things according to that
However, the chunk is still able to upload its phys triangles and generate its body, so that the body and triangles never have to leave it
But this is never done automatically and only done when the world tells it to
Kind of a mess but I can easily change it when I have to
*/
void World::buildData() {
	verts.clear();
	debug_points.clear();
	indirect.clear();
	info.clear();

	GLuint start_index = 0, end_index = 0;

	for (GLuint x = 0; x < WORLD_SIZE_X; x++) {
		for (GLuint y = 0; y < WORLD_SIZE_Y; y++) {
			for (GLuint z = 0; z < WORLD_SIZE_Z; z++) {

				const glm::vec3 coords = getChunkCoordsFloat(x, y, z);
	
				Chunk &chunk = chunks[x][y][z];

				if (chunk.vertsHaveChanged == true) {
					chunk.rebuildVerts();
					chunk.rebuildBody(getChunkCoordsFloat(x, y, z)); // I need to offset the phys body, make the world give the chunk its coords
				}

				end_index += chunks[x][y][z].addVertsTo(verts);
				(void)chunks[x][y][z].addPointsTo(debug_points);

				indirect.emplace_back(start_index, end_index - start_index);
				info.emplace_back(coords);

				start_index = end_index;
			}
		}
	}
}

SelectedBlockInfo World::getBlockInfo(const glm::ivec3 &position) {
	SelectedBlockInfo ret;

	// had weird innacuracies when values were not floats, like -7.15 + 8 == 0 but actualy was -7 + 8 == 1

	Chunk *chunk = &chunks // this gets ID of the chunk
		[static_cast<GLuint>((static_cast<GLfloat>(position.x) / CHUNK_SIZE_FLOAT) + (WORLD_SIZE_X_FLOAT / 2.0f))]
		[static_cast<GLuint>((static_cast<GLfloat>(position.y) / CHUNK_SIZE_FLOAT) + (WORLD_SIZE_Y_FLOAT / 2.0f))]
		[static_cast<GLuint>((static_cast<GLfloat>(position.z) / CHUNK_SIZE_FLOAT) + (WORLD_SIZE_Z_FLOAT / 2.0f))];

	ret.chunkID = chunk - &chunks[0][0][0];

	// this gets position inside of the chunk
	glm::u8vec3 pos;
	if (position.x < 0) {
		pos.x = (CHUNK_SIZE - (abs(position.x) % CHUNK_SIZE)) % CHUNK_SIZE; // weird math needed since 1) negative values 2) starts at -1 not 0. the last % feels bad, is only for when == CHUNK_SIZE to prevent resulting in CHUNK_SIZE. have to redo this math
	} else {
		pos.x = position.x % CHUNK_SIZE;
	}
	
	if (position.y < 0) {
		pos.y = (CHUNK_SIZE - (abs(position.y) % CHUNK_SIZE)) % CHUNK_SIZE; // weird math needed since 1) negative values 2) starts at -1 not 0. the last % feels bad, is only for when == CHUNK_SIZE to prevent resulting in CHUNK_SIZE. have to redo this math
	} else {
		pos.y = position.y % CHUNK_SIZE;
	}
	
	if (position.z < 0) {
		pos.z = (CHUNK_SIZE - (abs(position.z) % CHUNK_SIZE)) % CHUNK_SIZE; // weird math needed since 1) negative values 2) starts at -1 not 0. the last % feels bad, is only for when == CHUNK_SIZE to prevent resulting in CHUNK_SIZE. have to redo this math
	} else {
		pos.z = position.z % CHUNK_SIZE;
	}

	ret.local_pos = pos;
	ret.world_pos = position;
	ret.materialID = chunk->getVoxelAt(pos).material_id;
	ret._isEmpty = chunk->isVoxelEmptyAt(pos);

	return ret;
}

SelectedBlockInfo World::getSelectedBlock(const glm::vec3 &position, const glm::vec3 &lookPosition, GLfloat radius) {
	radius *= 2.0f; // to act as range
	int x = static_cast<int>(std::floor(position.x));
	int y = static_cast<int>(std::floor(position.y));
	int z = static_cast<int>(std::floor(position.z));

	const float dx = lookPosition.x;
	const float dy = lookPosition.y;
	const float dz = lookPosition.z;

	const int stepX = signum(dx);
	const int stepY = signum(dy);
	const int stepZ = signum(dz);

	float tMaxX = intbound(position.x, dx);
	float tMaxY = intbound(position.y, dy);
	float tMaxZ = intbound(position.z, dz);

	const float tDeltaX = stepX / dx;
	const float tDeltaY = stepY / dy;
	const float tDeltaZ = stepZ / dz;

	if (dx == 0 && dy == 0 && dz == 0) {
		// throw std::range_error("Raycast in zero direction!");
		return SelectedBlockInfo(-1, 0, 0, true, {}, {});
	}

	// will optimize this later
	char face[3] = {0, 0, 0};

	// constexpr int max_iter = 10; // to be removed later
	// int i = 0;

	// radius /= sqrt(dx*dx+dy*dy+dz*dz); // ?????????

	// put radius condition here?????????????????????
	while (
			// (stepX > 0 ? x < wx : x >= 0) &&
			// (stepY > 0 ? y < wy : y >= 0) &&
			// (stepZ > 0 ? z < wz : z >= 0) &&
			// (stepX < 0 ? x >= -wx : true) && // Check for negative x bounds
			// (stepY < 0 ? y >= -wy : true) && // Check for negative y bounds
			// (stepZ < 0 ? z >= -wz : true) && // Check for negative z bounds
			//(i < max_iter)) {
			(true)) {

		if (!(x < MIN_X || y < MIN_Y || z < MIN_Z || x > MAX_X || y > MAX_Y || z > MAX_Z)) {
			const glm::ivec3 coords = glm::ivec3(x, y, z);

			SelectedBlockInfo blockInfo = getBlockInfo(coords);


			if (! blockInfo.isEmpty()) {

				// printf("%d %d %d face %d %d %d\n", x, y, z, face[0], face[1], face[2]);
				// printf("detected a block at %d %d %d\n", coords.x, coords.y, coords.z);

				// will optimize this later
				if (face[0] == 1) blockInfo.normal = NORMAL_POS_X;
				else if (face[0] == -1) blockInfo.normal = NORMAL_NEG_X;

				if (face[1] == 1) blockInfo.normal = NORMAL_POS_Y;
				else if (face[1] == -1) blockInfo.normal = NORMAL_NEG_Y;

				if (face[2] == 1) blockInfo.normal = NORMAL_POS_Z;
				else if (face[2] == -1) blockInfo.normal = NORMAL_NEG_Z;
				return blockInfo;
			}
		}
		// else {
		// 	break;
		// }

		if (tMaxX < tMaxY) {
			if (tMaxX < tMaxZ) {
				if (tMaxX > radius) break;
				x += stepX;
				tMaxX += tDeltaX;
				face[0] = -stepX;
				face[1] = 0;
				face[2] = 0;
			} else {
				if (tMaxZ > radius) break;
				z += stepZ;
				tMaxZ += tDeltaZ;
				face[0] = 0;
				face[1] = 0;
				face[2] = -stepZ;
			}
		} else {
			if (tMaxY < tMaxZ) {
				if (tMaxY > radius) break;
				y += stepY;
				tMaxY += tDeltaY;
				face[0] = 0;
				face[1] = -stepY;
				face[2] = 0;
			} else {
				if (tMaxZ > radius) break;
				z += stepZ;
				tMaxZ += tDeltaZ;
				face[0] = 0;
				face[1] = 0;
				face[2] = -stepZ;
			}
		}


		// i++;

	}
	// nothing found within range
	return SelectedBlockInfo(-1, 0, 0, true, {}, {});
}

// void World::addSphere(const glm::vec3 &center, GLfloat radius) {
// 	// TODO this is currently implemented in the worst way possible

// 	GLubyte value = 0;
	
// 	for (GLuint x = 0; x < WORLD_SIZE_X; x++) {
// 		for (GLuint y = 0; y < WORLD_SIZE_Y; y++) {
// 			for (GLuint z = 0; z < WORLD_SIZE_Z; z++) {
// 				const glm::vec3 offset = getChunkCoordsFloat(x, y, z);

// 				for (GLuint cx = 0; cx < CHUNK_SIZE; cx++) {
// 					for (GLuint cy = 0; cy < CHUNK_SIZE; cy++) {
// 						for (GLuint cz = 0; cz < CHUNK_SIZE; cz++) {

// 							for (GLubyte corner = 0; corner < 8; corner++) {
// 								const glm::vec3 &final_position = LookupTable::corner_coords[corner] + glm::vec3(static_cast<GLfloat>(cx), static_cast<GLfloat>(cy), static_cast<GLfloat>(cz)) + offset;

// 								if ((final_position.x - center.x) * (final_position.x - center.x) + (final_position.y - center.y) * (final_position.y - center.y) + (final_position.z - center.z) * (final_position.z - center.z) <= radius * radius ) {
// 									value |= 1 << corner;
// 								}
// 							}

// 							if (value != 0x00) {
// 								chunks[x][y][z].insertVoxelAt(glm::uvec3(cx, cy, cz), Voxel(value, 0));
// 							}

// 							value = 0;
// 						}
// 					}
// 				}
// 			}
// 		}
// 	}
// }

void World::breakVoxelSphere(const SelectedBlockInfo &selectedInfo, GLfloat radius) {
	const glm::vec3 center = glm::vec3(selectedInfo.world_pos); //glm::vec3(getWorldCoords(selectedInfo.chunkID, selectedInfo.position));
	// const GLfloat radius_squared = radius * radius;

	// this constant is the size of the diagonal of a 1x1 cube (ie, the biggest possible distance inside of it)
	// sqrt(2 * x^2 + x^2), x = 1, sqrt(3)
	constexpr GLfloat diagonal = 1.73205080757f;

	// 2 spheres will be calculated, one that is big enough to make sure no chunk that should be considered is in fact not considered,
	// and a smaller one where all chunks within can be considered as being completely empty after the destruction
	const GLfloat big_radius   = radius + (diagonal * CHUNK_SIZE_FLOAT);
	const GLfloat small_radius = radius - (diagonal * CHUNK_SIZE_FLOAT);

	// if it is small enough, no chunk could ever possibli fit inside
	// > vs >= ???

	if (small_radius >= CHUNK_SIZE_FLOAT) {
		// calculate the bounding box and destroy every chunk inside it
		GLint min_x = glm::clamp(static_cast<GLint>(((center.x - small_radius) / CHUNK_SIZE_FLOAT) + (WORLD_SIZE_X_FLOAT / 2.0f)), 0, WORLD_SIZE_X - 1),
		      max_x = glm::clamp(static_cast<GLint>(((center.x + small_radius) / CHUNK_SIZE_FLOAT) + (WORLD_SIZE_X_FLOAT / 2.0f)), 0, WORLD_SIZE_X - 1),
			  min_y = glm::clamp(static_cast<GLint>(((center.y - small_radius) / CHUNK_SIZE_FLOAT) + (WORLD_SIZE_Y_FLOAT / 2.0f)), 0, WORLD_SIZE_Y - 1),
			  max_y = glm::clamp(static_cast<GLint>(((center.y + small_radius) / CHUNK_SIZE_FLOAT) + (WORLD_SIZE_Y_FLOAT / 2.0f)), 0, WORLD_SIZE_Y - 1),
			  min_z = glm::clamp(static_cast<GLint>(((center.z - small_radius) / CHUNK_SIZE_FLOAT) + (WORLD_SIZE_Z_FLOAT / 2.0f)), 0, WORLD_SIZE_Z - 1),
			  max_z = glm::clamp(static_cast<GLint>(((center.z + small_radius) / CHUNK_SIZE_FLOAT) + (WORLD_SIZE_Z_FLOAT / 2.0f)), 0, WORLD_SIZE_Z - 1);

		// printf("center: %f %f %f radius: %f\n", center.x, center.y, center.z, small_radius);
		// printf("x %d %d y %d %d z %d %d\n", min_x, max_x, min_y, max_y, min_z, max_z);

		for (GLint x = min_x; x <= max_x; x++) {
			for (GLint y = min_y; y <= max_y; y++) {
				for (GLint z = min_z; z <= max_z; z++) {
					chunks[x][y][z].destroyChunk();
				}
			}
		}
	}

	GLint min_x = glm::clamp(static_cast<GLint>(((center.x - big_radius) / CHUNK_SIZE_FLOAT) + (WORLD_SIZE_X_FLOAT / 2.0f)), 0, WORLD_SIZE_X - 1),
		  max_x = glm::clamp(static_cast<GLint>(((center.x + big_radius) / CHUNK_SIZE_FLOAT) + (WORLD_SIZE_X_FLOAT / 2.0f)), 0, WORLD_SIZE_X - 1),
		  min_y = glm::clamp(static_cast<GLint>(((center.y - big_radius) / CHUNK_SIZE_FLOAT) + (WORLD_SIZE_Y_FLOAT / 2.0f)), 0, WORLD_SIZE_Y - 1),
		  max_y = glm::clamp(static_cast<GLint>(((center.y + big_radius) / CHUNK_SIZE_FLOAT) + (WORLD_SIZE_Y_FLOAT / 2.0f)), 0, WORLD_SIZE_Y - 1),
		  min_z = glm::clamp(static_cast<GLint>(((center.z - big_radius) / CHUNK_SIZE_FLOAT) + (WORLD_SIZE_Z_FLOAT / 2.0f)), 0, WORLD_SIZE_Z - 1),
		  max_z = glm::clamp(static_cast<GLint>(((center.z + big_radius) / CHUNK_SIZE_FLOAT) + (WORLD_SIZE_Z_FLOAT / 2.0f)), 0, WORLD_SIZE_Z - 1);


	for (GLint x = min_x; x <= max_x; x++) {
		for (GLint y = min_y; y <= max_y; y++) {
			for (GLint z = min_z; z <= max_z; z++) {
				Chunk &chunk = chunks[x][y][z];
				if (! chunk.isDestroyed()) {
					// printf("considering chunk %d %d %d (%f %f %f)\n", x, y, z, getChunkCoordsFloat(x, y, z).x, getChunkCoordsFloat(x, y, z).y, getChunkCoordsFloat(x, y, z).z);
					chunk.breakSphere(center, radius * radius, getChunkCoordsFloat(x, y, z));
				}
			}
		}
	}
	// exit(1);
}

// TODO not optimized
void World::loadHeightMap(const std::string &path) {
	stbi_set_flip_vertically_on_load(true);
	constexpr int expected_width = WORLD_SIZE_X * CHUNK_SIZE;
	constexpr int expected_height = WORLD_SIZE_Z * CHUNK_SIZE;

	// offset that places world on the positive quadrants
	constexpr glm::ivec3 offset = glm::ivec3(((WORLD_SIZE_X / 2) * CHUNK_SIZE_CORNERS),
											 ((WORLD_SIZE_Y / 2) * CHUNK_SIZE_CORNERS),
											 ((WORLD_SIZE_Z / 2) * CHUNK_SIZE_CORNERS));

	int width, height, BPP;
	unsigned char *buffer =	stbi_load(path.c_str(), &width, &height, &BPP, 1);

	CRASH_IF(!buffer, "Error loading image");

	if (width != expected_width || height != expected_height) {
		Log::log(LOG_TYPE::WARN, std::string(__PRETTY_FUNCTION__),
			"image dimensions for " + std::string(path) + ": Expected " + std::to_string(expected_width) + " " + std::to_string(expected_height) + 
			" got " + std::to_string(width) + " " + std::to_string(height) + ". The image will be automatically resized");
		unsigned char * resized_buffer = (unsigned char*) malloc(expected_width * expected_height * 1); 
		stbir_resize_uint8_linear(buffer, width, height, 0, resized_buffer, expected_width, expected_height, 0, STBIR_1CHANNEL);

		height = expected_height;
		width = expected_width;

		// to simplify just pretend the buffer is the resized image
		free(buffer);
		buffer = resized_buffer;
	}

	for (GLuint x = 0; x < WORLD_SIZE_X; x++) {
		for (GLuint y = 0; y < WORLD_SIZE_Y; y++) {
			for (GLuint z = 0; z < WORLD_SIZE_Z; z++) {
				Chunk &chunk = chunks[x][y][z];

				// chunk needs to know where it is
				chunk.generate(getChunkCoords(x, y, z), buffer, width, offset, (x + z) % 2);
			}
		}
	}

	free(buffer);
}
