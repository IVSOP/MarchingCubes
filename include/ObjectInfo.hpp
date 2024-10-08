// cursed but whatever I needed to fix circular dependencies

#ifndef OBJECTINFO_HPP
#define OBJECTINFO_HPP

#include "types.hpp"
#include "Vertex.hpp"
#include "CustomVec.hpp"
#include "Phys.hpp"

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

#endif
