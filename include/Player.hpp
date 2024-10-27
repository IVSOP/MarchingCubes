#ifndef PLAYER_H
#define PLAYER_H

#include <entt.hpp>
#include "Components.hpp"
#include "Phys.hpp"


// I will store a reference to the registry for simplicity, idk If I like this
class Player {
public:

	// entity info
	// entt::registry &registry;
	// entt::entity player_entity;

	// after a physics update, this recalculates the needed vectors
	void postTick();
	Direction dir; // Direction has angles and other things which are not needed but make my life easier
	Movement mov;

	// phys info
	// TODO the description of these references does not sound good, seems like a shared_ptr, change to something else
	JPH::Ref<JPH::Character> physCharacter;
	// JPH::Ref<JPH::CharacterVirtual>	physCharacterVirtual;
	JPH::RefConst<JPH::Shape> standingShape; // shape player when he is standing
	// Player input
	JPH::Vec3 mControlInput = JPH::Vec3::sZero();
	bool mJump = false;
	bool mWasJump = false;
	bool mSwitchStance = false;
	bool mWasSwitchStance = false;
	const float	playerHeight = 1.35f;
	const float	playerRadius = 0.3f;

	// ???? WHAT WHY IS THIS HERE
	// List of active characters in the scene so they can collide
	JPH::CharacterVsCharacterCollisionSimple characterVsCharacterCollision;

	Player();
	// Player(entt::registry &registry);
	~Player();

	void setupPhys(const Position &position, const glm::vec3 &lookat);

	void move(Camera_Movement type, float deltaTime);
	void look(float xoffset, float yoffset);
	glm::mat4 getViewMatrix();
	void speedUp(bool speedup);

	// most of these are only valid after postick is called!!!!!
	Position  getPos() const; // not a reference for complicated reasons
	// this is only valid after postick is called!!!!!
	Direction &getDir();
	Movement  &getMov();
	glm::vec3 getRotation() const;
	glm::vec3 getUpVector(); // TODO make this const
	glm::vec3 getVelocity() const;

	static void noclipCallback(void *player, const void *data);
	void noclip(bool activation);

	// Player(std::ifstream &file);
	// void saveTo(std::ofstream &file) const;
private:
};

#endif
