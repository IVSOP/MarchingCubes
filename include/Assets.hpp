#ifndef ASSETS_H
#define ASSETS_H

#include "common.hpp"
#include "Vertex.hpp"
#include "VertContainer.hpp"

#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags

#define ASSETS_FOLDER "assets/"

#include "Phys.hpp"

struct GameObject {
	VertContainer<ModelVertex> verts;
	std::vector<GLuint> indices; // use vertcontainer???
	JPH::TriangleList phys_triangles;
	// this is a weird hack I don't like it
	JPH::Body *phys_body = nullptr;

	GameObject() : verts(1) {} // cursed
	GameObject(std::size_t vert_cap) : verts(vert_cap) {}
	GameObject(VertContainer<ModelVertex> verts) : verts(verts) {}
	~GameObject() = default;
};

// uses assimp to import things
class Importer {
public:
	// returns unique_ptr to make it clear it is the caller's responsibility to free it
	static std::unique_ptr<GameObject> load(const std::string &name);

	// adds object to vector of other objects
	static void load(const std::string &name, std::vector<GameObject> &objs);

	// for debug
	static void dumpMetadata(const std::string &name);
};


#endif
