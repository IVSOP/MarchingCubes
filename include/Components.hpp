#ifndef COMPONENTS_HPP
#define COMPONENTS_HPP

#include "common.hpp"
#include "Phys.hpp"
#include "Audio.hpp"

// IMPORTANT!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// entt went against my way of doing things
// solved by prrohibiting structs from being moved or copied
// TODO this might make things inneficient
// only apply this to non trivial structs!!!
#define NON_COPYABLE_AND_NON_MOVABLE(TypeName)                    \
    TypeName(const TypeName&) = delete;                                          \
    TypeName& operator=(const TypeName&) = delete;                               \
    TypeName(TypeName&&) = delete;                                               \
    TypeName& operator=(TypeName&&) = delete;

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

	// NON_COPYABLE_AND_NON_MOVABLE(Position)
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

	// NON_COPYABLE_AND_NON_MOVABLE(Direction)
};

// movement modifiers
struct Movement {
	GLfloat speed;
	bool speedup;

	Movement()
		: speed(1.0f), speedup(false) {}

	Movement(GLfloat speed, bool speedup)
		: speed(speed), speedup(speedup) {}

	// NON_COPYABLE_AND_NON_MOVABLE(Movement)
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

	JPH::Vec3 getPosition() const {
		return body->GetPosition();
	}
	JPH::Quat getRotation() const {
		return body->GetRotation();
	}

	void activate() {
		Phys::activateBody(body);
	}

	void setUserData(UserData data) {
		Phys::setUserData(body, data);
	}

	NON_COPYABLE_AND_NON_MOVABLE(Physics)
};

// to render, info is obtained using this ID
// TODO this is more flexible, but if it is too slow will have to restructure the entire pipeline again to make a faster way of getting all entities of the same id
struct Render {
	uint32_t object_id;

	Render() : object_id(0) {};
	Render(uint32_t object_id) : object_id(object_id) {}
	~Render() = default;

	// NON_COPYABLE_AND_NON_MOVABLE(Render)
};

// TODO lots of hardcoded things
// also ref vs *
// and const ref vs refconst
struct PhysicsCharacter {
	// body is in Physics component
	// TODO can I just use pointers here??
	JPH::Ref<JPH::Character> physCharacter;
	const float maxSeparationDistance = 0.1f;

	// actually not my fault that this is inconsistent with Physics
	// still TODO move this into physics
	PhysicsCharacter(JPH::RefConst<JPH::Shape> shape, const JPH::Vec3 &translation, const JPH::Quat &rotation, UserData data) {
		{
			JPH::Ref<JPH::CharacterSettings> settings = new JPH::CharacterSettings();
			settings->mMaxSlopeAngle = JPH::DegreesToRadians(45.0f);
			settings->mLayer = Layers::MOVING;
			settings->mShape = shape;
			settings->mUp = JPH::Vec3(0.0f, 1.0f, 0.0f);
			settings->mFriction = 0.5f;
			settings->mSupportingVolume = JPH::Plane(JPH::Vec3::sAxisY(), -0.5f); // Accept contacts that touch the lower sphere of the capsule
			// TODO the player direction here is wrong, but fine on the camera. player needs to have direction from jolt and not entt // it already does, remove this?
			// TODO set this rotation so that player looks to some point
			physCharacter = new JPH::Character(settings, translation, rotation, data.data, Phys::getPhysSystem());
			physCharacter->AddToPhysicsSystem(JPH::EActivation::Activate);
		}
	}
	~PhysicsCharacter() {
		physCharacter->RemoveFromPhysicsSystem();
	}

	void postTick() {
		physCharacter->PostSimulation(maxSeparationDistance);
	}

	glm::mat4 getTransform() const {
		return Phys::getCharacterTransform(physCharacter);
	}

	JPH::Vec3 getPosition() const {
		return physCharacter->GetPosition();
	}
	JPH::Quat getRotation() const {
		return physCharacter->GetRotation();
	}

	NON_COPYABLE_AND_NON_MOVABLE(PhysicsCharacter)
};

// for now, entities have 1 source each
struct AudioComponent {
	// creates source with a buffer from audio with this filename
	AudioComponent(const std::string &filename) {
		const Audio::Buffer &buff = Audio::ALContext::createBufferFromWavIfNotExists(filename);

		source.setLoop();

		source.setBuffer(buff);
	}

	void play() const {
		source.play();
	}

	void pause() const {
		source.pause();
	}

	void setPosition(const glm::vec3 &pos) const {
		source.setPosition(pos);
	}

	void setGain(const ALfloat gain) const {
		source.setGain(gain);
	}

	~AudioComponent() = default;

	Audio::Source source;

	NON_COPYABLE_AND_NON_MOVABLE(AudioComponent)
};

struct Selected {
	Selected() = default;
	~Selected() = default;
};

#endif
