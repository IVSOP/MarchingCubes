#ifndef ASSETS_H
#define ASSETS_H

#include "common.hpp"
#include "Vertex.hpp"
#include "CustomVec.hpp"
#include "Phys.hpp"
#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags
#include "embree.hpp"
#include "ObjectInfo.hpp"

#define ASSETS_FOLDER "assets/"


// TODO make a separate struct and map for hitboxes. for now, if model exists the same hitbox is always used

// uses assimp to import things
class Assets {
public:

	// for now since I don't have an editor I need a human readable format so I'll use json for the hitboxes
	// vecs are represented like a list of floats
	// see Phys
	// blender uses wxyz, I use xyzw
	// blender has y-up
	// basically I need to do treat blender abcd as bdca

	// adds object to vector of other objects
	static void load(const std::string &model, const std::string &hitbox, CustomVec<GameObject> &objs);
	// TODO make this, and make it create a convex hull from all the points
	static void load(const std::string &model, CustomVec<GameObject> &objs);
	// instead of using the actual model, calculates how it would look as marching cubes geometry
	static void loadMarchingCubes(const std::string &model, CustomVec<MarchingCubesObject> &objs, uint32_t len_x, uint32_t len_y, uint32_t len_z);

	// for debug
	static void dumpMetadata(const std::string &name);
};


#endif
