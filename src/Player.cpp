#include "Player.hpp"

Player::Player(entt::registry &registry, const Position &position, const glm::vec3 &lookatPoint)
						// register the player entity
	: registry(registry), player_entity(registry.create())
{
	// register the needed components
	registry.emplace<Position>(player_entity, position);
	registry.emplace<Direction>(player_entity, Direction::lookat(position.pos, glm::vec3(0.0f, 1.0f, 0.0f), lookatPoint));
	registry.emplace<Movement>(player_entity, 10.0f, false);
	registry.emplace<Physics>(player_entity);

	// // make the shape a capsule
	// standingShape = JPH::RotatedTranslatedShapeSettings(JPH::Vec3(0, 0.5f * playerHeight + playerRadius, 0), JPH::Quat::sIdentity(), new JPH::CapsuleShape(0.5f * playerHeight, playerRadius)).Create().Get();

	// // Create Character
	// {
	// 	JPH::CharacterSettings settings;
	// 	settings.mLayer = Layers::MOVING;
	// 	settings.mShape = standingShape;
	// 	settings.mSupportingVolume = JPH::Plane(JPH::Vec3::sAxisY(), -playerRadius); // Accept contacts that touch the lower sphere of the capsule
	// 	physCharacter = new JPH::Character(&settings, JPH::Vec3(position.pos.x, position.pos.y, position.pos.z), JPH::Quat::sIdentity(), 0, Phys::physics_system.get());
	// 	physCharacter->AddToPhysicsSystem();
	// }

	// // Create CharacterVirtual
	// {
	// 	JPH::CharacterVirtualSettings settings;
	// 	settings.mShape = standingShape;
	// 	settings.mSupportingVolume = JPH::Plane(JPH::Vec3::sAxisY(), -playerRadius); // Accept contacts that touch the lower sphere of the capsule
	// 	// why is this position different??????
	// 	physCharacterVirtual = new JPH::CharacterVirtual(&settings, JPH::Vec3(position.pos.x - 2.0f, position.pos.y, position.pos.z), JPH::Quat::sIdentity(), 0, Phys::physics_system.get());
	// 	physCharacterVirtual->SetCharacterVsCharacterCollision(&characterVsCharacterCollision);
	// 	characterVsCharacterCollision.Add(physCharacterVirtual);
	// }
}

void Player::move(Camera_Movement direction, float deltaTime) {
	// Position &pos = registry.get<Position>(player_entity);

	const Direction &dir = registry.get<Direction>(player_entity);

	const Movement &mov = registry.get<Movement>(player_entity);

	Physics &phys = registry.get<Physics>(player_entity);


	float velocity; // depends on deltatime
	if (mov.speedup) {
		velocity = 10 * mov.speed * deltaTime;
	} else {
		velocity = mov.speed * deltaTime;
	}

	switch(direction) {
		case(FORWARD):
			// normalize this too?????
			phys.vel += dir.front * velocity;
			break;
		case(BACKWARD):
			// normalize this too?????
			phys.vel -= dir.front * velocity;
			break;

		case(FRONT):
			// normalize needed since it would get different speeds for different Y values
			// contas manhosas sao para em vez de ir para a frente manter-se no mesmo plano (ex minecraft voar ao carregar no W nunca sobe nem desce)
			phys.vel += glm::normalize(dir.front * (glm::vec3(1.0f, 1.0f, 1.0f) - dir.worldup)) * velocity;
			break;
		case(BACK):
			// normalize needed since it would get different speeds for different Y values
			// contas manhosas sao para em vez de ir para a frente manter-se no mesmo plano (ex minecraft voar ao carregar no W nunca sobe nem desce)
			phys.vel -= glm::normalize(dir.front * (glm::vec3(1.0f, 1.0f, 1.0f) - dir.worldup)) * velocity;
			break;
		case(LEFT):
			phys.vel -= dir.right * velocity;
			break;
		case(RIGHT):
			phys.vel += dir.right * velocity;
			break;
		case(UP):
			phys.vel += glm::normalize(dir.up * dir.worldup) * velocity;
			break;
		case(DOWN):
			phys.vel -= glm::normalize(dir.up * dir.worldup) * velocity;
			break;
	}
}

void Player::look(float xoffset, float yoffset) {
	GLfloat MouseSensitivity = 0.1f; // hardcoded for now

	Direction &dir = registry.get<Direction>(player_entity);


	xoffset *= MouseSensitivity;
	yoffset *= MouseSensitivity;

	dir.yaw   += xoffset;
	dir.pitch += yoffset;

	// make sure that when pitch is out of bounds, screen doesn't get flipped
	if (true)
	{
		if (dir.pitch > 89.0f)
			dir.pitch = 89.0f;
		if (dir.pitch < -89.0f)
			dir.pitch = -89.0f;
	}

	// update Front, Right and Up Vectors using the updated Euler angles
	dir.updateVectors();

}

void Player::speedUp(bool speedup) {
	registry.get<Movement>(player_entity).speedup = speedup;
}

glm::mat4 Player::getViewMatrix() {
	const Position &pos = registry.get<Position>(player_entity);
	const Direction &dir = registry.get<Direction>(player_entity);

	return glm::lookAt(pos.pos, pos.pos + dir.front, dir.worldup);
}

Position  &Player::getPos() {
	return registry.get<Position>(player_entity);
}

Direction &Player::getDir() {
	return registry.get<Direction>(player_entity);
}

Movement  &Player::getMov() {
	return registry.get<Movement>(player_entity);
}


// Player::Player(std::ifstream &file)
// : camera(file)
// { }

// void Player::saveTo(std::ofstream &file) const {
// 	camera.saveTo(file);
// }
