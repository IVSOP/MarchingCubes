#include "Player.hpp"

Player::Player()
	: dir(), mov(10.0f, false)
{
	// TODO what a mess
	dir.up = glm::vec3(0.0f, 1.0f, 0.0f);
	dir.worldup = glm::vec3(0.0f, 1.0f, 0.0f);

	// register the needed components
	// registry.emplace<Position>(player_entity, position); // in physics
	// registry.emplace<Direction>(player_entity, Direction::lookat(position.pos, glm::vec3(0.0f, 1.0f, 0.0f), lookatPoint));
	// registry.emplace<Movement>(player_entity, 10.0f, false);
	// registry.emplace<Physics>(player_entity);
}

Player::~Player() {
	physCharacter->RemoveFromPhysicsSystem();
}

void Player::postTick() {
	dir.front = getRotation();
	dir.updateVectors();


	/// ???????? wtf is going on here
	// normalizing after setting y to 0 makes it so the player is turning vertically but the y component is 0 wtf??????
	// in debug mode this throws an assertion, here it just works
	// I don't even care

	JPH::Quat rot_jph = JPH::Quat(dir.front.x, dir.front.y, dir.front.z, 1.0f).Normalized();
	// rot_jph.SetY(0.0f);
	physCharacter->SetRotation(rot_jph);
	// printf("%f %f %f\n", rot_jph.GetX(), rot_jph.GetY(), rot_jph.GetZ());
}

// lookatPoint ignored for now, got lazy translating the lookatpoint into a vector
void Player::setupPhys(const Position &position, const glm::vec3 &lookatPoint) {
	// make the shape a capsule
	standingShape = JPH::RotatedTranslatedShapeSettings(JPH::Vec3(0, 0.5f * playerHeight + playerRadius, 0), JPH::Quat::sIdentity(), new JPH::CapsuleShape(0.5f * playerHeight, playerRadius)).Create().Get();

	// Create Character
	{
		JPH::Ref<JPH::CharacterSettings> settings = new JPH::CharacterSettings(); // TODO also see this ref, seems bad
		settings->mMaxSlopeAngle = JPH::DegreesToRadians(45.0f);
		settings->mLayer = Layers::MOVING;
		settings->mShape = standingShape;
		settings->mUp = JPH::Vec3(0.0f, 1.0f, 0.0f); // TODO do not hardcode this
		settings->mFriction = 0.5f;
		settings->mSupportingVolume = JPH::Plane(JPH::Vec3::sAxisY(), -playerRadius); // Accept contacts that touch the lower sphere of the capsule
		// TODO the player direction here is wrong, but fine on the camera. player needs to have direction from jolt and not entt // it already does, remove this?
		// TODO set this rotation so that player looks to some point
		physCharacter = new JPH::Character(settings, JPH::Vec3(position.pos.x, position.pos.y, position.pos.z), JPH::Quat::sIdentity(), 0, Phys::getPhysSystem());
		physCharacter->AddToPhysicsSystem(JPH::EActivation::Activate);
	}

	// // Create CharacterVirtual
	// {
	// 	JPH::CharacterVirtualSettings settings;
	// 	settings.mShape = standingShape;
	// 	settings.mSupportingVolume = JPH::Plane(JPH::Vec3::sAxisY(), -playerRadius); // Accept contacts that touch the lower sphere of the capsule
	// 	// why is this position different??????
	// 	physCharacterVirtual = new JPH::CharacterVirtual(&settings, JPH::Vec3(position.pos.x - 2.0f, position.pos.y, position.pos.z), JPH::Quat::sIdentity(), 0, Phys::getPhysSystem());
	// 	physCharacterVirtual->SetCharacterVsCharacterCollision(&characterVsCharacterCollision);
	// 	characterVsCharacterCollision.Add(physCharacterVirtual);
	// }
}


		// // Update velocity and apply gravity
		// Vec3 velocity;
		// if (mAnimatedCharacterVirtual->GetGroundState() == CharacterVirtual::EGroundState::OnGround)
		// 	velocity = Vec3::sZero();
		// else
		// 	velocity = mAnimatedCharacterVirtual->GetLinearVelocity() * mAnimatedCharacter->GetUp() + mPhysicsSystem->GetGravity() * inParams.mDeltaTime;
		// velocity += Sin(mTime) * cCharacterVelocity;
		// mAnimatedCharacterVirtual->SetLinearVelocity(velocity);

		// // Move character
		// CharacterVirtual::ExtendedUpdateSettings update_settings;
		// mAnimatedCharacterVirtual->ExtendedUpdate(inParams.mDeltaTime,
		// 	mPhysicsSystem->GetGravity(),
		// 	update_settings,
		// 	mPhysicsSystem->GetDefaultBroadPhaseLayerFilter(Layers::MOVING),
		// 	mPhysicsSystem->GetDefaultLayerFilter(Layers::MOVING),
		// 	{ },
		// 	{ },
		// 	*mTempAllocator);

void Player::move(Camera_Movement direction, float deltaTime) {
	// movement speed
	// const Movement &mov = registry.get<Movement>(player_entity);

	// JPH::Vec3 velocity;
	// // if on ground, do nothing
	// if (physCharacter->GetGroundState() == JPH::Character::EGroundState::OnGround) {
	// 	velocity = JPH::Vec3::sZero();
	// } else {
	// 	// use up from character or from the camera
	// 	velocity = mAnimatedCharacterVirtual->GetLinearVelocity() * physCharacter->GetUp() + Phys::getPhysSystem()->GetGravity() * deltaTime;
	// }

	// TODO fix this mess
	JPH::Vec3 current_velocity = physCharacter->GetLinearVelocity();

	float speed; // depends on deltatime
	if (mov.speedup) {
		speed = 10 * mov.speed * deltaTime;
	} else {
		speed = mov.speed * deltaTime;
	}

	glm::vec3 mov_dir(0.0f);
	switch(direction) {
		case(FORWARD):
			// normalize this too?????
			mov_dir = dir.front;
			break;
		case(BACKWARD):
			// normalize this too?????
			mov_dir = - dir.front;
			break;

		case(FRONT):
			// normalize needed since it would get different speeds for different Y values
			// contas manhosas sao para em vez de ir para a frente manter-se no mesmo plano (ex minecraft voar ao carregar no W nunca sobe nem desce)
			mov_dir = glm::normalize(dir.front * (glm::vec3(1.0f, 1.0f, 1.0f) - dir.worldup));
			break;
		case(BACK):
			// normalize needed since it would get different speeds for different Y values
			// contas manhosas sao para em vez de ir para a frente manter-se no mesmo plano (ex minecraft voar ao carregar no W nunca sobe nem desce)
			mov_dir = - glm::normalize(dir.front * (glm::vec3(1.0f, 1.0f, 1.0f) - dir.worldup));
			break;
		case(LEFT):
			mov_dir = - dir.right;
			break;
		case(RIGHT):
			mov_dir = dir.right;
			break;
		case(UP):
			mov_dir = glm::normalize(dir.worldup); // dir.up * dir.worldup); // why * up?????????
			break;
		case(DOWN):
			mov_dir = - glm::normalize(dir.worldup); // dir.up * dir.worldup); // why * up?????????
			break;
	}

	// cur + (movement * speed)
	JPH::Vec3 new_velocity = current_velocity + (JPH::Vec3(mov_dir.x, mov_dir.y, mov_dir.z) * speed);

	// if (!new_velocity.IsNearZero() || !physCharacter->IsSupported()) {
	// 	new_velocity.SetY(current_velocity.GetY());
	// }

	// Update the velocity
	physCharacter->SetLinearVelocity(new_velocity);
	// printf("velocity was %f %f %f, is now %f %f %f\n", current_velocity.GetX(), current_velocity.GetY(), current_velocity.GetZ(), new_velocity.GetX(), new_velocity.GetY(), new_velocity.GetZ());
}

void Player::look(float xoffset, float yoffset) {
	GLfloat MouseSensitivity = 0.1f; // hardcoded for now

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

	// JPH::Quat rot_jph = JPH::Quat(dir.front.x, dir.front.y, dir.front.z, 1.0f).Normalized();
	// !!! IMPORTANT to avoid problems, for now the player object only rotates horizontally
	// rot_jph.SetY(0.0f);
	// physCharacter->SetRotation(rot_jph);
}

void Player::speedUp(bool speedup) {
	// registry.get<Movement>(player_entity).speedup = speedup;
	mov.speedup = speedup;
}

glm::mat4 Player::getViewMatrix() {
	const glm::vec3 pos = getPos().pos;

	return glm::lookAt(pos, pos + dir.front, dir.worldup);
}

Position Player::getPos() const {
	// return registry.get<Position>(player_entity);
	JPH::Vec3 pos_jph = physCharacter->GetPosition();
	const glm::vec3 res = glm::vec3(pos_jph.GetX(), pos_jph.GetY(), pos_jph.GetZ());
	return Position(res);
}

Direction &Player::getDir() {
	return dir;
}

Movement &Player::getMov() {
	// return registry.get<Movement>(player_entity);
	return mov;
}

glm::vec3 Player::getRotation() const {
	JPH::Quat rot_jph = physCharacter->GetRotation();

	return glm::vec3(rot_jph.GetX(), rot_jph.GetY(), rot_jph.GetZ());
}

glm::vec3 Player::getUpVector() {
	return getDir().up;
}


// Player::Player(std::ifstream &file)
// : camera(file)
// { }

// void Player::saveTo(std::ofstream &file) const {
// 	camera.saveTo(file);
// }
