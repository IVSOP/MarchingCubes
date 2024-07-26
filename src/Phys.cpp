#include "Phys.hpp"

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
	Phys::temp_allocator = std::make_unique<TempAllocatorImpl>(10 * 1024 * 1024);

	// We need a job system that will execute physics jobs on multiple threads. Typically
	// you would implement the JobSystem interface yourself and let Jolt Physics run on top
	// of your own job scheduler. JobSystemThreadPool is an example implementation.
	Phys::job_system = std::make_unique<JobSystemThreadPool>(cMaxPhysicsJobs, cMaxPhysicsBarriers, std::thread::hardware_concurrency() - 1);

	// This is the max amount of rigid bodies that you can add to the physics system. If you try to add more you'll get an error.
	// Note: This value is low because this is a simple test. For a real project use something in the order of 65536.
	const uint cMaxBodies = 1024;

	// This determines how many mutexes to allocate to protect rigid bodies from concurrent access. Set it to 0 for the default settings.
	const uint cNumBodyMutexes = 0;

	// This is the max amount of body pairs that can be queued at any time (the broad phase will detect overlapping
	// body pairs based on their bounding boxes and will insert them into a queue for the narrowphase). If you make this buffer
	// too small the queue will fill up and the broad phase jobs will start to do narrow phase work. This is slightly less efficient.
	// Note: This value is low because this is a simple test. For a real project use something in the order of 65536.
	const uint cMaxBodyPairs = 1024;

	// This is the maximum size of the contact constraint buffer. If more contacts (collisions between bodies) are detected than this
	// number then these contacts will be ignored and bodies will start interpenetrating / fall through the world.
	// Note: This value is low because this is a simple test. For a real project use something in the order of 10240.
	const uint cMaxContactConstraints = 1024;

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

JPH::Body *Phys::createBody(const TriangleList &triangles) {
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
	if (body == nullptr) {
		exit(5);
	}

	bodyInterface.AddBody(body->GetID(), EActivation::DontActivate);
	// TODO delete body


	// Vec3 a = meshShape->GetCenterOfMass();
	// printf("%f %f %f\n", a.GetX(), a.GetY(), a.GetZ());
	// exit(1);

	return body;
}

JPH::Body *Phys::createBody(const TriangleList &triangles, const glm::vec3 &coords) {
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
	if (body == nullptr) {
		exit(5);
	}

	bodyInterface.AddBody(body->GetID(), EActivation::DontActivate);
	// TODO delete body


	// Vec3 a = meshShape->GetCenterOfMass();
	// printf("%f %f %f\n", a.GetX(), a.GetY(), a.GetZ());
	// exit(1);

	return body;
}

JPH::Body *Phys::createBodyWithNormals(const TriangleList &triangles, const glm::vec3 &coords, const Vec3 *normals) {
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
	if (body == nullptr) {
		exit(5);
	}

	bodyInterface.AddBody(body->GetID(), EActivation::DontActivate);
	// TODO delete body


	// Vec3 a = meshShape->GetCenterOfMass();
	// printf("%f %f %f\n", a.GetX(), a.GetY(), a.GetZ());
	// exit(1);

	return body;
}

// THIS SEGFAULTS , DO NOT USE
// pretty sure there is no way to make it ever work
JPH::Body *Phys::createEmptyBody() {
	BodyInterface &bodyInterface = getBodyInterface();

	JPH::Shape *shape = new MeshShape();

	JPH::Vec3 terrainPosition = JPH::Vec3::sZero();
	JPH::Quat terrainRotation = JPH::Quat::sIdentity();

	// this can receive either shape or shape settings, and only needs a pointer
	// need to figure out what is the optimal way to do things
	// can receive shape * or settings *
	JPH::BodyCreationSettings bodySettings(shape, terrainPosition, terrainRotation, JPH::EMotionType::Static, Layers::NON_MOVING);

	// could also receive indices
	Body *body = bodyInterface.CreateBody(bodySettings);

	// TODO remove this
	if (body == nullptr) {
		exit(5);
	}

	bodyInterface.AddBody(body->GetID(), EActivation::DontActivate);
	// TODO delete body


	// Vec3 a = meshShape->GetCenterOfMass();
	// printf("%f %f %f\n", a.GetX(), a.GetY(), a.GetZ());
	// exit(1);

	return body;
}

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
