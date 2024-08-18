#ifndef ASSETS_H
#define ASSETS_H

#include "common.hpp"
#include "Vertex.hpp"
#include "CustomVec.hpp"
#include "Phys.hpp"
#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags

#define ASSETS_FOLDER "assets/"


// TODO make a separate struct and map for hitboxes. for now, if model exists the same hitbox is always used

// info about an object, allowing to render it and make its physics object
struct GameObject {
	CustomVec<ModelVertex> verts;
	std::vector<GLuint> indices; // TODO use CustomVec???

	JPH::RefConst<JPH::Shape> phys_shape;

	GameObject() : verts(1) {} // cursed
	GameObject(std::size_t vert_cap) : verts(vert_cap) {}
																			 // can I do this??? what happens to the underlying data
	// GameObject(CustomVec<ModelVertex> verts, std::vector<GLuint> indices) : verts(verts), indices(indices) {}
	~GameObject() = default;
};

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
	static void load(const std::string &model, CustomVec<GameObject> &objs) { }

	// for debug
	static void dumpMetadata(const std::string &name);
};


#endif
