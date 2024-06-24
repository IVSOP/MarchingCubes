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
/* starting from the left (from FF in 0xFF000000), that is, the most significant bits
32 bits {
	5 - pos x
	5 - pos y
	5 - pos z
	4 - edge A
	4 - edge B
	4 - edge C
	5 - material (not enough, but good for now)
}
*/
struct Vertex {
	uint32_t data;

	Vertex() = default;

	// messy conversions
	Vertex(const glm::u8vec3 &local_pos, const glm::u8vec3 &edges, GLint material_id)
	{
		data = ((local_pos.x << 27) & 0xF8000000) | ((local_pos.y << 22) & 0x07C00000) | ((local_pos.z << 17) & 0x003E0000)
			 | ((edges.x << 13) & 0x0001E000) | ((edges.y << 9) & 0x00001E00) | ((edges.z << 5) & 0x000001E0)
			 | (material_id & 0x0000001F);
	}
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
