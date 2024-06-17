#ifndef VERTEX_H
#define VERTEX_H

#include <iostream>
#include <glm/glm.hpp>

#include <GL/glew.h> // GLfloat

struct Vertex { // struct of vertices as sent to the GPU. position is relative to the chunk, and the normal can only be 0, 1, ..., 5
	glm::vec3 coords;
	glm::vec2 tex_coords;
	GLint material_id;

	Vertex() = default;

	Vertex(glm::vec3 coords, glm::vec2 tex_coords, GLint material_id)
	: coords(coords), tex_coords(tex_coords), material_id(material_id) { }
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
