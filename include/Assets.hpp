#ifndef ASSETS_H
#define ASSETS_H

#include "common.hpp"
#include "Vertex.hpp"
#include "VertContainer.hpp"
#include "Phys.hpp"
#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags

#define ASSETS_FOLDER "assets/"


// TODO make a separate struct and map for hitboxes. for now, if model exists the same hitbox is always used

// info about an object, allowing to render it and make its physics object
struct GameObject {
	VertContainer<ModelVertex> verts;
	std::vector<GLuint> indices; // TODO use vertcontainer???

	// TODO keep this in the stack or delete the body if kept this way
	// also don't forget to remove it from phys system
	JPH::RefConst<JPH::Shape> phys_shape;

	GameObject() : verts(1) {} // cursed
	GameObject(std::size_t vert_cap) : verts(vert_cap) {}
	GameObject(VertContainer<ModelVertex> verts, std::vector<GLuint> indices) : verts(verts), indices(indices) {}
	~GameObject() = default; // TODO
};

// uses assimp to import things
class Assets {
public:

	// the two load functions receive a hitbox file as input
	// for now since I don't have an editor I need a human readable format so I'll use json
	// vecs are represented like a list of floats
	// see Phys
	// blender uses wxyz, I use xyzw
	// blender has y-up
	// basically I need to do treat blender abcd as bdca

	// https://stackoverflow.com/questions/62253972/is-it-safe-to-reference-a-value-in-unordered-map
	// pray that this is true for pointers as well
	// static const GameObject *load(const std::string &model, const std::string &hitbox);

	// adds object to vector of other objects
	static void load(const std::string &model, const std::string &hitbox, std::vector<GameObject> &objs);

	// for debug
	static void dumpMetadata(const std::string &name);
};


#endif
