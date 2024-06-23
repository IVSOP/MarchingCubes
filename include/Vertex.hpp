#ifndef VERTEX_H
#define VERTEX_H

#include <iostream>
#include <glm/glm.hpp>

#include <GL/glew.h> // GLfloat

struct Point {
	glm::vec3 pos;

	Point(const glm::vec3 &pos)
	: pos(pos) { }
};

// naming is hard, this data is actually applied per instance
struct Vertex {
	glm::ivec3 local_pos;
	glm::ivec3 edges;
	GLint material_id;

	Vertex() = default;

	Vertex(const glm::ivec3 &local_pos, const glm::ivec3 &edges, GLint material_id)
	: local_pos(local_pos), edges(edges), material_id(material_id) { }
};

struct AxisVertex { // to draw the axis
	glm::vec4 coords = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	glm::vec4 color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

	AxisVertex() = default;
	AxisVertex(float x, float y, float z, float R, float G, float B) : coords(x, y, z, 1.0f), color(R, G, B, 1.0f) {}
};

struct ViewportVertex { // if needed, to draw on the entire viewport
	glm::vec4 coords = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	glm::vec2 tex_coord = glm::vec2(0.0f, 0.0f);

	ViewportVertex(float x, float y, float z, float tex_x, float tex_y) : coords(x, y, z, 1.0f), tex_coord(tex_x, tex_y) {}
};

#endif //CG_VERTEX_H
