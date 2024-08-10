#include "Phys.hpp"
#include "Crash.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>


using namespace JPH;

std::unique_ptr<PhysicsSystem> Phys::phys_system = nullptr;
std::unique_ptr<JPH::TempAllocatorImpl> Phys::temp_allocator = nullptr;
std::unique_ptr<JPH::JobSystem> Phys::job_system = nullptr;

void Phys::setup_phys() {
	// Register allocation hook. In this example we'll just let Jolt use malloc / free but you can override these if you want (see Memory.h).
	// This needs to be done before any other Jolt function is called.
	RegisterDefaultAllocator();

	// Install trace and assert callbacks
	Trace = TraceImpl;
	JPH_IF_ENABLE_ASSERTS(AssertFailed = AssertFailedImpl;)

	// Create a factory, this class is responsible for creating instances of classes based on their name or hash and is mainly used for deserialization of saved data.
	// It is not directly used in this example but still required.
	// what the fuck is going on here
	Factory::sInstance = new Factory();

	// Register all physics types with the factory and install their collision handlers with the CollisionDispatch class.
	// If you have your own custom shape types you probably need to register their handlers with the CollisionDispatch before calling this function.
	// If you implement your own default material (PhysicsMaterial::sDefault) make sure to initialize it before this function or else this function will create one for you.
	RegisterTypes();

	// We need a temp allocator for temporary allocations during the physics update. We're
	// pre-allocating 10 MB to avoid having to do allocations during the physics update.
	// B.t.w. 10 MB is way too much for this example but it is a typical value you can use.
	// If you don't want to pre-allocate you can also use TempAllocatorMalloc to fall back to
	// malloc / free.
	Phys::temp_allocator = std::make_unique<TempAllocatorImpl>(100 * 1024 * 1024);

	// We need a job system that will execute physics jobs on multiple threads. Typically
	// you would implement the JobSystem interface yourself and let Jolt Physics run on top
	// of your own job scheduler. JobSystemThreadPool is an example implementation.
	Phys::job_system = std::make_unique<JobSystemThreadPool>(cMaxPhysicsJobs, cMaxPhysicsBarriers, std::thread::hardware_concurrency() - 1);

	// This is the max amount of rigid bodies that you can add to the physics system. If you try to add more you'll get an error.
	// Note: This value is low because this is a simple test. For a real project use something in the order of 65536.
	const uint cMaxBodies = 65536;

	// This determines how many mutexes to allocate to protect rigid bodies from concurrent access. Set it to 0 for the default settings.
	const uint cNumBodyMutexes = 0;

	// This is the max amount of body pairs that can be queued at any time (the broad phase will detect overlapping
	// body pairs based on their bounding boxes and will insert them into a queue for the narrowphase). If you make this buffer
	// too small the queue will fill up and the broad phase jobs will start to do narrow phase work. This is slightly less efficient.
	// Note: This value is low because this is a simple test. For a real project use something in the order of 65536.
	const uint cMaxBodyPairs = 65536;

	// This is the maximum size of the contact constraint buffer. If more contacts (collisions between bodies) are detected than this
	// number then these contacts will be ignored and bodies will start interpenetrating / fall through the world.
	// Note: This value is low because this is a simple test. For a real project use something in the order of 10240.
	const uint cMaxContactConstraints = 10240;

	// Create mapping table from object layer to broadphase layer
	// Note: As this is an interface, PhysicsSystem will take a reference to this so this instance needs to stay alive!
	BPLayerInterfaceImpl *broad_phase_layer_interface = new BPLayerInterfaceImpl(); // TODO free this

	// Create class that filters object vs broadphase layers
	// Note: As this is an interface, PhysicsSystem will take a reference to this so this instance needs to stay alive!
	ObjectVsBroadPhaseLayerFilterImpl *object_vs_broadphase_layer_filter = new ObjectVsBroadPhaseLayerFilterImpl(); // TODO delete this

	// Create class that filters object vs object layers
	// Note: As this is an interface, PhysicsSystem will take a reference to this so this instance needs to stay alive!
	ObjectLayerPairFilterImpl *object_vs_object_layer_filter = new ObjectLayerPairFilterImpl(); // TODO free this

	// Now we can create the actual physics system.
	Phys::phys_system = std::make_unique<PhysicsSystem>();
	Phys::phys_system->Init(cMaxBodies, cNumBodyMutexes, cMaxBodyPairs, cMaxContactConstraints, *broad_phase_layer_interface, *object_vs_broadphase_layer_filter, *object_vs_object_layer_filter);

		// // A body activation listener gets notified when bodies activate and go to sleep
		// // Note that this is called from a job so whatever you do here needs to be thread safe.
		// // Registering one is entirely optional.
		// MyBodyActivationListener body_activation_listener;
		// physics_system.SetBodyActivationListener(&body_activation_listener);

		// // A contact listener gets notified when bodies (are about to) collide, and when they separate again.
		// // Note that this is called from a job so whatever you do here needs to be thread safe.
		// // Registering one is entirely optional.
		// MyContactListener contact_listener;
		// physics_system.SetContactListener(&contact_listener);

	// The main way to interact with the bodies in the physics system is through the body interface. There is a locking and a non-locking
	// variant of this. We're going to use the locking version (even though we're not planning to access bodies from multiple threads)
	// BodyInterface body_interface = physics_system->GetBodyInterface();
}

BodyInterface &Phys::getBodyInterface() { return Phys::phys_system->GetBodyInterface(); }

JPH::Body *Phys::createTerrain(const TriangleList &triangles) {
	BodyInterface &bodyInterface = getBodyInterface();

	JPH::MeshShapeSettings meshShapeSettings = JPH::MeshShapeSettings(triangles);
	meshShapeSettings.SetEmbedded();
	JPH::ShapeRefC meshShape = meshShapeSettings.Create().Get(); // this vs JPH::MeshShapeSettings* ????? TODO

	JPH::Vec3 terrainPosition = JPH::Vec3::sZero();
	JPH::Quat terrainRotation = JPH::Quat::sIdentity();

	// this can receive either shape or shape settings, and only needs a pointer
	// need to figure out what is the optimal way to do things
	// can receive shape * or settings *
	JPH::BodyCreationSettings bodySettings(meshShape, terrainPosition, terrainRotation, JPH::EMotionType::Static, Layers::NON_MOVING);

	// could also receive indices
	Body *body = bodyInterface.CreateBody(bodySettings);

	// TODO remove this
	CRASH_IF(body == nullptr, "No more bodies available");

	bodyInterface.AddBody(body->GetID(), EActivation::DontActivate);


	// Vec3 a = meshShape->GetCenterOfMass();
	// printf("%f %f %f\n", a.GetX(), a.GetY(), a.GetZ());
	// exit(1);

	return body;
}

JPH::Body *Phys::createTerrain(const TriangleList &triangles, const glm::vec3 &coords) {
	BodyInterface &bodyInterface = getBodyInterface();

	JPH::MeshShapeSettings meshShapeSettings = JPH::MeshShapeSettings(triangles);
	meshShapeSettings.SetEmbedded();
	JPH::ShapeRefC meshShape = meshShapeSettings.Create().Get(); // this vs JPH::MeshShapeSettings* ????? TODO

	JPH::Vec3 terrainPosition = JPH::Vec3(coords.x, coords.y, coords.z);
	JPH::Quat terrainRotation = JPH::Quat::sIdentity();

	// this can receive either shape or shape settings, and only needs a pointer
	// need to figure out what is the optimal way to do things
	// can receive shape * or settings *
	JPH::BodyCreationSettings bodySettings(meshShape, terrainPosition, terrainRotation, JPH::EMotionType::Static, Layers::NON_MOVING);

	// could also receive indices
	Body *body = bodyInterface.CreateBody(bodySettings);

	// TODO remove this
	CRASH_IF(body == nullptr, "No more bodies available");

	bodyInterface.AddBody(body->GetID(), EActivation::DontActivate);


	// Vec3 a = meshShape->GetCenterOfMass();
	// printf("%f %f %f\n", a.GetX(), a.GetY(), a.GetZ());
	// exit(1);

	return body;
}

// commented out since I didnt push the changes I made to jolt
JPH::Body *Phys::createTerrainWithNormals(const TriangleList &triangles, const glm::vec3 &coords, const Vec3 *normals) {
	// BodyInterface &bodyInterface = getBodyInterface();

	// JPH::MeshShapeSettings meshShapeSettings = JPH::MeshShapeSettings(triangles);
	// meshShapeSettings.SetEmbedded();
	// JPH::ShapeRefC meshShape = meshShapeSettings.Create().Get(); // this vs JPH::MeshShapeSettings* ????? TODO

	// JPH::Vec3 terrainPosition = JPH::Vec3(coords.x, coords.y, coords.z);
	// JPH::Quat terrainRotation = JPH::Quat::sIdentity();

	// // this can receive either shape or shape settings, and only needs a pointer
	// // need to figure out what is the optimal way to do things
	// // can receive shape * or settings *
	// JPH::BodyCreationSettings bodySettings(meshShape, terrainPosition, terrainRotation, JPH::EMotionType::Static, Layers::NON_MOVING);

	// // could also receive indices
	// Body *body = bodyInterface.CreateBody(bodySettings);

	// // TODO remove this
	// if (body == nullptr) {
	// 	exit(5);
	// }

	// bodyInterface.AddBody(body->GetID(), EActivation::DontActivate);
	// // TODO delete body


	// // Vec3 a = meshShape->GetCenterOfMass();
	// // printf("%f %f %f\n", a.GetX(), a.GetY(), a.GetZ());
	// // exit(1);

	// return body;
	return nullptr;
}

JPH::Body *Phys::createBody(const TriangleList &triangles) {
	float mass = 100.0f;
	BodyInterface &bodyInterface = getBodyInterface();

	JPH::MeshShapeSettings meshShapeSettings = JPH::MeshShapeSettings(triangles);
	meshShapeSettings.SetEmbedded();
	JPH::ShapeRefC meshShape = meshShapeSettings.Create().Get(); // this vs JPH::MeshShapeSettings* ????? TODO

	JPH::Vec3 terrainPosition = JPH::Vec3::sZero();
	JPH::Quat terrainRotation = JPH::Quat::sIdentity();

	// this can receive either shape or shape settings, and only needs a pointer
	// need to figure out what is the optimal way to do things
	// can receive shape * or settings *
	JPH::BodyCreationSettings bodySettings(meshShape, terrainPosition, terrainRotation, JPH::EMotionType::Dynamic, Layers::MOVING);
	bodySettings.mOverrideMassProperties = EOverrideMassProperties::CalculateInertia;
	bodySettings.mMassPropertiesOverride.mMass = mass;

	// could also receive indices
	Body *body = bodyInterface.CreateBody(bodySettings);

	// TODO remove this
	CRASH_IF(body == nullptr, "No more bodies available");

	bodyInterface.AddBody(body->GetID(), EActivation::DontActivate);


	// Vec3 a = meshShape->GetCenterOfMass();
	// printf("%f %f %f\n", a.GetX(), a.GetY(), a.GetZ());
	// exit(1);

	return body;
}

JPH::Body *Phys::createBody(const TriangleList &triangles, const glm::vec3 &coords) {
	float mass = 100.0f;
	BodyInterface &bodyInterface = getBodyInterface();

	JPH::MeshShapeSettings meshShapeSettings = JPH::MeshShapeSettings(triangles);
	meshShapeSettings.SetEmbedded();
	JPH::ShapeRefC meshShape = meshShapeSettings.Create().Get(); // this vs JPH::MeshShapeSettings* ????? TODO

	JPH::Vec3 terrainPosition = JPH::Vec3(coords.x, coords.y, coords.z);
	JPH::Quat terrainRotation = JPH::Quat::sIdentity();

	// this can receive either shape or shape settings, and only needs a pointer
	// need to figure out what is the optimal way to do things
	// can receive shape * or settings *
	JPH::BodyCreationSettings bodySettings(meshShape, terrainPosition, terrainRotation, JPH::EMotionType::Dynamic, Layers::MOVING);
	bodySettings.mOverrideMassProperties = EOverrideMassProperties::CalculateInertia;
	bodySettings.mMassPropertiesOverride.mMass = mass;

	// could also receive indices
	Body *body = bodyInterface.CreateBody(bodySettings);

	// TODO remove this
	CRASH_IF(body == nullptr, "No more bodies available");

	bodyInterface.AddBody(body->GetID(), EActivation::DontActivate);


	// Vec3 a = meshShape->GetCenterOfMass();
	// printf("%f %f %f\n", a.GetX(), a.GetY(), a.GetZ());
	// exit(1);

	return body;
}



JPH::Body *Phys::createBodyFromJson(const json &data) {

	JPH::Ref<JPH::StaticCompoundShapeSettings> compound_shape = new JPH::StaticCompoundShapeSettings;

	// TODO lots of error checking, this assumes all values exist and have the correct type
	// when using AddShape(), the offsets and rotations are relative to the compound shape
	// I could also apply them when creating the shapes themselves
	// for now I'll leave it in AddShape
	float position_x, position_y, position_z;
	float rotation_x, rotation_y, rotation_z, rotation_w;

	for (const auto &shape_data : data) {
		position_x = shape_data["position"][0].get<float>();
		position_y = shape_data["position"][1].get<float>();
		position_z = shape_data["position"][2].get<float>();

		rotation_x = shape_data["rotation"][0].get<float>();
		rotation_y = shape_data["rotation"][1].get<float>();
		rotation_z = shape_data["rotation"][2].get<float>();
		rotation_w = shape_data["rotation"][3].get<float>();

		// use shapes or shape settings?????????????????
		const std::string type = shape_data["type"].get<std::string>();

		// cant use switch :(

		if (type == "box") {
			float scale_x, scale_y, scale_z;
			scale_x = shape_data["scale"][0].get<float>();
			scale_y = shape_data["scale"][1].get<float>();
			scale_z = shape_data["scale"][2].get<float>();

			compound_shape->AddShape(
				Vec3(position_x, position_y, position_z),
				Quat(rotation_x, rotation_y, rotation_z, rotation_w),
				new BoxShape(Vec3(scale_x, scale_y, scale_z))
			);
		} else if (type == "sphere") {
			float radius = shape_data["radius"].get<float>();

			compound_shape->AddShape(
				Vec3(position_x, position_y, position_z),
				Quat(rotation_x, rotation_y, rotation_z, rotation_w),
				new SphereShape(radius)
			);
		} else if (type == "cylinder") {
			float radius = shape_data["radius"].get<float>();
			float height = shape_data["height"].get<float>();

			compound_shape->AddShape(
				Vec3(position_x, position_y, position_z),
				Quat(rotation_x, rotation_y, rotation_z, rotation_w),
				new CylinderShape(height, radius)
			);
		} else {
				CRASH_IF(true, "Invalid hitbox shape type");
		}
	}

	BodyCreationSettings bodySettings(compound_shape, Vec3::sZero(), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING);
	BodyInterface &bodyInterface = getBodyInterface();
	Body* body = bodyInterface.CreateBody(bodySettings);
	bodyInterface.AddBody(body->GetID(), EActivation::DontActivate);


	return body;
}

void Phys::activateBody(const JPH::Body *body) {
	BodyInterface &bodyInterface = getBodyInterface();

	bodyInterface.ActivateBody(body->GetID());
}

void Phys::addBodyToSystem(const JPH::Body *body) {
	BodyInterface &bodyInterface = getBodyInterface();

	bodyInterface.AddBody(body->GetID(), EActivation::DontActivate);
}

glm::mat4 Phys::getBodyTransform(const JPH::Body *body) {
	JPH::RVec3 position = body->GetPosition();
	JPH::Quat rotation = body->GetRotation();

	glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(position.GetX(), position.GetY(), position.GetZ()));
	glm::quat glmQuat(rotation.GetW(), rotation.GetX(), rotation.GetY(), rotation.GetZ());
	glm::mat4 rotationMatrix = glm::toMat4(glmQuat); // TODO use glm::rotate instead???

	return translationMatrix * rotationMatrix;
}

// // THIS SEGFAULTS , DO NOT USE
// // pretty sure there is no way to make it ever work
// JPH::Body *Phys::createEmptyBody() {
// 	BodyInterface &bodyInterface = getBodyInterface();

// 	JPH::Shape *shape = new MeshShape();

// 	JPH::Vec3 terrainPosition = JPH::Vec3::sZero();
// 	JPH::Quat terrainRotation = JPH::Quat::sIdentity();

// 	// this can receive either shape or shape settings, and only needs a pointer
// 	// need to figure out what is the optimal way to do things
// 	// can receive shape * or settings *
// 	JPH::BodyCreationSettings bodySettings(shape, terrainPosition, terrainRotation, JPH::EMotionType::Static, Layers::NON_MOVING);

// 	// could also receive indices
// 	Body *body = bodyInterface.CreateBody(bodySettings);

// 	// TODO remove this
// 	CRASH_IF(body == nullptr, "No more bodies available");

// 	bodyInterface.AddBody(body->GetID(), EActivation::DontActivate);
// 	// TODO delete body


// 	// Vec3 a = meshShape->GetCenterOfMass();
// 	// printf("%f %f %f\n", a.GetX(), a.GetY(), a.GetZ());
// 	// exit(1);

// 	return body;
// }

void Phys::setBodyMeshShape(Body *body, const TriangleList &triangles) {
	BodyInterface &bodyInterface = getBodyInterface();

	JPH::MeshShapeSettings meshShapeSettings = JPH::MeshShapeSettings(triangles);
	meshShapeSettings.SetEmbedded();
	JPH::ShapeRefC meshShape = meshShapeSettings.Create().Get(); // this vs JPH::MeshShapeSettings* ????? TODO

	// false since center of mass does not matter for terrain
	bodyInterface.SetShape(body->GetID(), meshShape, false, EActivation::DontActivate);
	// TODO delete body


	// Vec3 a = meshShape->GetCenterOfMass();
	// printf("%f %f %f\n", a.GetX(), a.GetY(), a.GetZ());
	// exit(1);
}

void Phys::destroyBody(JPH::Body *body) {
	BodyInterface &bodyInterface = getBodyInterface();

	bodyInterface.RemoveBody(body->GetID());
	bodyInterface.DestroyBody(body->GetID());
}

PhysicsSystem *Phys::getPhysSystem() {
	return Phys::phys_system.get();
}

TempAllocatorImpl *Phys::getTempAllocator() {
	return Phys::temp_allocator.get();
}

JobSystem *Phys::getJobSystem() {
	return Phys::job_system.get();
}



// Callback for traces, connect this to your own trace function if you have one
void TraceImpl(const char *inFMT, ...)
{
	// Format the message
	va_list list;
	va_start(list, inFMT);
	char buffer[1024];
	vsnprintf(buffer, sizeof(buffer), inFMT, list);
	va_end(list);

	// Print to the TTY
	std::cout << buffer << std::endl;
}

#ifdef JPH_ENABLE_ASSERTS

	// Callback for asserts, connect this to your own assert handler if you have one
	bool AssertFailedImpl(const char *inExpression, const char *inMessage, const char *inFile, JPH::uint inLine)
	{
		// Print to the TTY
		std::cout << inFile << ":" << inLine << ": (" << inExpression << ") " << (inMessage != nullptr? inMessage : "") << std::endl;

		// Breakpoint
		return true;
	};

#endif // JPH_ENABLE_ASSERTS
