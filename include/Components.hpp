#ifndef COMPONENTS_HPP
#define COMPONENTS_HPP

#include "common.hpp"
#include "Phys.hpp"

struct Position {
	glm::vec3 pos;

	Position(const glm::vec3 &pos)
		: pos(pos) {}
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
	glm::vec3 vel;
	glm::vec3 force;
	GLfloat mass = 1.0f;

	Physics()
		: vel(0.0f), force(0.0f) {}

	Physics(const glm::vec3 &vel, const glm::vec3 &force)
		: vel(vel), force(force) {}

	constexpr void addGravity() {
		force += mass * glm::vec3(0.0f, -9.8f, 0.0f);
	}

	constexpr void applyToPosition(glm::vec3 &position, GLfloat deltatime) {
		vel += (force / mass) * deltatime;
		position += vel * deltatime;
		// position += deltatime * (vel + deltatime * accel * 0.5f);

		force = glm::vec3(0.0f);
	}

	// constexpr void applyFriction(GLfloat coeff) {
		// got lazy, needed to calculate x and z force components etc
		// force -= coeff * glm::vec3();
	// }

	constexpr void slowDown(GLfloat coeff, GLfloat deltatime) {
		vel -= (vel * coeff) * deltatime;
		// force -= (force * coeff) * deltatime;
	}
};

#endif
