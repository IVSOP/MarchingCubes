#ifndef PLAYER_H
#define PLAYER_H

#include <entt.hpp>
#include "Components.hpp"

// I will store a reference to the registry for simplicity, idk If I like this
class Player {
public:
	entt::registry &registry;
	entt::entity player_entity;

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
