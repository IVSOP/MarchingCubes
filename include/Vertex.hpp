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

	glm::u8vec3 getLocalPos() const {
		glm::u8vec3 res;

		res.x = (data >> 27) & 0x0000001F;
		res.y = (data >> 22) & 0x0000001F;
		res.z = (data >> 17) & 0x0000001F;

		return res;
	}

	glm::u8vec3 getEdges() const {
		glm::u8vec3 res;

		res.x = (data >> 13) & 0x0000000F;
		res.y = (data >> 9)  & 0x0000000F;
		res.z = (data >> 5)  & 0x0000000F;

		return res;
	}
};

struct AxisVertex { // to draw the axis
	glm::vec3 coords = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec4 color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

	AxisVertex() = default;
	AxisVertex(float x, float y, float z, float R, float G, float B) : coords(x, y, z), color(R, G, B, 1.0f) {}
	AxisVertex(float x, float y, float z, float R, float G, float B, float A) : coords(x, y, z), color(R, G, B, A) {}
	AxisVertex(const glm::vec3 &coords, const glm::vec4 &color) : coords(coords), color(color) {}
};

struct ViewportVertex { // if needed, to draw on the entire viewport
	glm::vec4 coords = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	glm::vec2 tex_coord = glm::vec2(0.0f, 0.0f);

	ViewportVertex(float x, float y, float z, float tex_x, float tex_y) : coords(x, y, z, 1.0f), tex_coord(tex_x, tex_y) {}
};

struct ModelVertex {
	glm::vec3 coords = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 normal;
	glm::vec2 uv;
	ModelVertex(const glm::vec3 &coords, const glm::vec3 &normal, const glm::vec2 &uv) : coords(coords), normal(normal), uv(uv) {}
};

using PhysVertex = AxisVertex;

// struct PhysVertex {
// 	glm::vec3 coords = glm::vec3(0.0f);
// 	glm::vec4 color = glm::vec4(1.0f);

// 	PhysVertex(const glm::vec3 &coords, const glm::vec4 &color) : coords(coords),color(color) {}
// };

#endif //CG_VERTEX_H
