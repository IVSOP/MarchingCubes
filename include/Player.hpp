#ifndef PLAYER_H
#define PLAYER_H

#include <entt.hpp>
#include "Components.hpp"
#include "Phys.hpp"
#include <Jolt/Physics/Character/Character.h>
#include <Jolt/Physics/Character/CharacterVirtual.h>

// I will store a reference to the registry for simplicity, idk If I like this
class Player {
public:

	// entity info
	entt::registry &registry;
	entt::entity player_entity;

	// phys info
	// TODO the description of these references does not sound good
	JPH::Ref<JPH::Character> physCharacter;
	JPH::Ref<JPH::CharacterVirtual>	physCharacterVirtual;
	JPH::RefConst<JPH::Shape> standingShape; // shape player when he is standing
	// Player input
	JPH::Vec3 mControlInput = JPH::Vec3::sZero();
	bool mJump = false;
	bool mWasJump = false;
	bool mSwitchStance = false;
	bool mWasSwitchStance = false;
	const float	playerHeight = 1.35f;
	const float	playerRadius = 0.3f;

	// List of active characters in the scene so they can collide
	JPH::CharacterVsCharacterCollisionSimple characterVsCharacterCollision;

	Player() = delete;
	// idk how I feel about this, but this class registers the new player entity in the registry
	Player(entt::registry &registry, const Position &position, const glm::vec3 &lookat);
	~Player() = default;

	void move(Camera_Movement type, float deltaTime);
	void look(float xoffset, float yoffset);
	glm::mat4 getViewMatrix();
	void speedUp(bool speedup);

	Position  &getPos();
	Direction &getDir();
	Movement  &getMov();

	// Player(std::ifstream &file);
	// void saveTo(std::ofstream &file) const;
};

#endif
