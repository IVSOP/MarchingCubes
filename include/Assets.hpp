#ifndef ASSETS_H
#define ASSETS_H

#include "common.hpp"
#include "Vertex.hpp"
#include "VertContainer.hpp"

#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags

#define ASSETS_FOLDER "assets/"

struct GameObject {
	VertContainer<SimpleVertex> verts;

	GameObject() = delete;
	GameObject(std::size_t vert_cap) : verts(vert_cap) {}
	GameObject(VertContainer<SimpleVertex> verts) : verts(verts) {}
	~GameObject() = default;
};

// uses assimp to import things
class Importer {
public:
	// for now returns only the model at the root node
	// returns unique_ptr to make it clear it is the caller's responsibility to free it, automatically or not
	static std::unique_ptr<GameObject> load(const std::string &name);
};


#endif
