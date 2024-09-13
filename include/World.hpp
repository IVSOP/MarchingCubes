#ifndef WORLD_H
#define WORLD_H

#include "common.hpp"
// #include "Chunk.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/norm.hpp> // distance2

#include <entt.hpp>
#include "Phys.hpp"
#include <unordered_map>
#include "Assets.hpp"
#include "Frustum.hpp"
#include "Files.hpp"

// #include "zlib/zlib.h"

class World {
public:
	Chunk chunks[WORLD_SIZE_X][WORLD_SIZE_Y][WORLD_SIZE_Z]; // this order can be changed, need to test it for performance

	CustomVec<Vertex> verts; // so I dont have to constantly alloc and free
	CustomVec<Point> debug_points;
	std::vector<IndirectData> indirect;
	std::vector<ChunkInfo> info;
	entt::registry entt_registry;


	constexpr Chunk &get(const glm::uvec3 &position) {
		return chunks[position.x][position.y][position.z];
	}

	CustomVec<Vertex> &getVerts() {
		return verts;
	}

	CustomVec<Point> &getPoints() {
		return debug_points;
	}

	std::vector<IndirectData> &getIndirect() {
		return indirect;
	}

	std::vector<ChunkInfo> &getInfo() {
		return info;
	}

	void buildData(const Frustum &frustum);

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
		loadModels();
		// the default constructor of each chunk creates an empty body
		// or it would if that was possible
	}

	World(FileHandler &file);

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

	// SelectedBlockInfo is what the caller will have, and it contains all the information needed to do this
	// TODO optimize, sometimes gettint outside chunks might not be needed
	void breakVoxel(const SelectedBlockInfo &selectedInfo) {
		// Chunk &chunk = getChunkByID(selectedInfo.chunkID);
		// chunk.breakVoxelAt(selectedInfo.local_pos);
		constexpr GLfloat diagonal = 1.73205080757f;
		breakVoxelSphere(selectedInfo, diagonal);
	}

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
	// also see https://en.wikipedia.org/wiki/Midpoint_circle_algorithm
	// in the clamps, do +/- 1 to final result instead of using the small and big radius
	void breakVoxelSphere(const SelectedBlockInfo &selectedInfo, GLfloat radius);
	void addVoxelShpere(const SelectedBlockInfo &selectedInfo, GLfloat radius);

	void loadHeightMap(const std::string &path);

	// loads model and stores its info, returning the object_id
	uint32_t loadModel(const std::string &name, const std::string &hitbox_name);
	uint32_t loadModel(const std::string &name);
	uint32_t loadModelMarchingCubes(const std::string &name, uint32_t len_x, uint32_t len_y, uint32_t len_z);
	JPH::Body *createBodyFromID(uint32_t id, const JPH::Vec3 &position, const JPH::Quat &rotation);
	// creates a renderable physics entity internally, given an object_id
	entt::entity spawn(uint32_t object_id, const JPH::Vec3 &translation, const JPH::Quat &rotation);
	entt::entity spawnCharacter(uint32_t object_id, const JPH::Vec3 &translation, const JPH::Quat &rotation);
	void spawnMarchingCubes(uint32_t object_id, const glm::ivec3 &pos); // TODO accept rotation, create entt entity

	// vector of pair<render info for a single instance, array of transforms of all entities to be drawn>
	const std::vector<std::pair<GameObject *, std::vector<glm::mat4>>> getEntitiesToDraw(const Frustum &frustum);
	const std::vector<std::pair<GameObject *, std::vector<glm::mat4>>> getSelectedEntities();

	void save(FileHandler &file);

	// loads all models needed, TODO change this
	void loadModels();

private:
	// while I do have an ECS, it is dumb to have N entities share 1 model and make them have a component with the vertices or something
	// I want to render all entities of the same model all at once using instancing
	// I need a fast way to get all entities that share some model, but this is kind of incompatible with an ECS system
	// TODO improve this, for now every single frame I loop over ALL entities and group them by their object_id, cannot come up with a good solution right now
	// !!!!!!!!!! DO NOT USE std::vector AS IT CALLS DESTRUCTOR WHEN DOING REALLOCATIONS
	CustomVec<GameObject> objects_info = CustomVec<GameObject>(1);

	CustomVec<MarchingCubesObject> mc_objects_info = CustomVec<MarchingCubesObject>(1);

	// // to allow spawning entities using the model name instead of the ID, but will be slower
	// std::unordered_map<std::string, uint32_t> name_to_id;
};


#endif
