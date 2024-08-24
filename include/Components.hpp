#ifndef COMPONENTS_HPP
#define COMPONENTS_HPP

#include "common.hpp"
#include "Phys.hpp"

// TODO make this whole thing including serialization not suck, lots of hardcoded values and strings
enum class Component : uint32_t {
	Position = 0,
	Direction = 1,
	Movement = 2,
	Physics = 3,
	Render = 4
};

struct Position {
	glm::vec3 pos;

	Position() : pos(0.0f) {}

	Position(const glm::vec3 &pos)
		: pos(pos) {}
	~Position() = default;
};

// TODO break into other components, only done this way for now as a test to use camera more easily
struct Direction {
	GLfloat pitch, yaw;
	glm::vec3 front;
	glm::vec3 worldup;

	// saved so they can be used to move camera more easily
	glm::vec3 up;
	glm::vec3 right;

	Direction()
		: pitch(0.0f), yaw(0.0f), front(0.0f), worldup(0.0f), up(0.0f), right(0.0f) {}

	Direction(GLfloat pitch, GLfloat yaw, const glm::vec3 &front, const glm::vec3 &worldup, const glm::vec3 &up, const glm::vec3 &right)
		: pitch(pitch), yaw(yaw), front(front), worldup(worldup), up(up), right(right) {}

	static Direction lookat(const glm::vec3 &pos, const glm::vec3 &worldup, const glm::vec3 &lookatPoint) {
		glm::vec3 tempVec = glm::normalize(lookatPoint - pos);
		GLfloat yaw = glm::degrees(atan2(tempVec.z, tempVec.x));
		GLfloat pitch = glm::degrees(atan2(tempVec.y, sqrt((tempVec.z * tempVec.z) + (tempVec.x * tempVec.x))));

		glm::vec3 front;
        front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        front.y = sin(glm::radians(pitch));
        front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
		front = glm::normalize(front);

		glm::vec3 right = glm::normalize(glm::cross(front, worldup));
        glm::vec3 up    = glm::normalize(glm::cross(right, front));

        return Direction(pitch, yaw, front, worldup, up, right);
	}

	void updateVectors() {
		front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        front.y = sin(glm::radians(pitch));
        front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
		front = glm::normalize(front);

		right = glm::normalize(glm::cross(front, worldup));
        up    = glm::normalize(glm::cross(right, front));
	}
};

// movement modifiers
struct Movement {
	GLfloat speed;
	bool speedup;

	Movement()
		: speed(1.0f), speedup(false) {}

	Movement(GLfloat speed, bool speedup)
		: speed(speed), speedup(speedup) {}
};

struct Physics {
	JPH::Body *body;

	Physics() : body(nullptr) {}
	Physics(JPH::Body *body) : body(body) {}

	~Physics() {
		Phys::destroyBody(body);
	}

	// TODO consider only sending rotation and translation to the gpu
	glm::mat4 getTransform() const {
		return Phys::getBodyTransform(body);
	}
};

// to render, info is obtained using this ID
// TODO this is more flexible, but if it is too slow will have to restructure the entire pipeline again to make a faster way of getting all entities of the same id
struct Render {
	uint32_t object_id;

	Render() : object_id(0) {};
	Render(uint32_t object_id) : object_id(object_id) {}
	~Render() = default;
};

// this is bad since many entities share this exact thing, no reason to do this
// struct Render {
// 	// this is a bit of a hack, info actually comes from Assets, these are just references
// 	const VertContainer<ModelVertex> &verts;
// 	const std::vector<GLuint> &indices;

// 	Render() = delete;
// 	Render(const VertContainer<ModelVertex> &verts, const std::vector<GLuint> &indices) : verts(verts), indices(indices) {}
// 	~Render() = default;
// };

#endif
