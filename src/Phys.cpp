#include "Phys.hpp"
#include "Crash.hpp"
#include "Profiling.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>


using namespace JPH;

std::unique_ptr<PhysicsSystem> Phys::phys_system = nullptr;
std::unique_ptr<JPH::TempAllocatorImpl> Phys::temp_allocator = nullptr;
std::unique_ptr<JPH::JobSystem> Phys::job_system = nullptr;
std::unique_ptr<PhysRenderer> Phys::phys_renderer = nullptr;

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

	phys_renderer = std::make_unique<PhysRenderer>();
	DebugRenderer::sInstance = phys_renderer.get();
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



// TODO when using AddShape I use new since that is what the example did
// does it ever get freed???????????????? wtf is going on here
// TODO return compound shape?
RefConst<JPH::Shape> Phys::createShapeFromJson(const json &data) {

	JPH::Ref<JPH::StaticCompoundShapeSettings> compound_shape = new JPH::StaticCompoundShapeSettings;

	// TODO lots of error checking, this assumes all values exist and have the correct type
	// when using AddShape(), the offsets and rotations are relative to the compound shape
	// I could also apply them when creating the shapes themselves
	// for now I'll leave it in AddShape

	// TODO vec3
	float position_x, position_y, position_z;
	JPH::Quat rotation;

	for (const auto &shape_data : data) {
		position_x = shape_data["position"][0].get<float>();
		position_y = shape_data["position"][1].get<float>();
		position_z = shape_data["position"][2].get<float>();

		rotation = JPH::Quat(
			shape_data["rotation"][0].get<float>(),
			shape_data["rotation"][1].get<float>(),
			shape_data["rotation"][2].get<float>(),
			shape_data["rotation"][3].get<float>()
		).Normalized();

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
				rotation,
				new BoxShape(Vec3(scale_x, scale_y, scale_z))
			);
		} else if (type == "sphere") {
			float radius = shape_data["radius"].get<float>();

			compound_shape->AddShape(
				Vec3(position_x, position_y, position_z),
				rotation,
				new SphereShape(radius)
			);
		} else if (type == "cylinder") {
			float radius = shape_data["radius"].get<float>();
			float height = shape_data["height"].get<float>();

			compound_shape->AddShape(
				Vec3(position_x, position_y, position_z),
				rotation,
				new CylinderShape(height, radius)
			);
		} else {
				CRASH_IF(true, "Invalid hitbox shape type");
		}
	}

	const ShapeSettings::ShapeResult shapeResult = compound_shape->Create();

    CRASH_IF(shapeResult.HasError(), "Error creating compound shape: " + std::string(shapeResult.GetError()));

	// RefConst<JPH::Shape> res = shapeResult.Get();
	return shapeResult.Get();

	// BodyCreationSettings bodySettings(compound_shape, Vec3::sZero(), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING);
	// BodyInterface &bodyInterface = getBodyInterface();
	// Body* body = bodyInterface.CreateBody(bodySettings);
	// bodyInterface.AddBody(body->GetID(), EActivation::DontActivate);
}

JPH::RefConst<JPH::Shape> Phys::createConvexHull(const CustomVec<ModelVertex> &verts, const std::vector<GLuint> &indices) {
	// TODO optimize this
	JPH::Array<JPH::Vec3> jph_verts;
	for (unsigned int i = 0; i < indices.size(); i++) {
		// why not reference??
		const glm::vec3 &vert = verts[indices[i]].coords;
		jph_verts.emplace_back(JPH::Vec3(vert.x, vert.y, vert.z));
	}

	JPH::Ref<JPH::ConvexHullShapeSettings> convex_hull = new JPH::ConvexHullShapeSettings(jph_verts);
	const ShapeSettings::ShapeResult shapeResult = convex_hull->Create();

    CRASH_IF(shapeResult.HasError(), "Error creating convex hull shape: " + std::string(shapeResult.GetError()));

	// RefConst<JPH::Shape> res = shapeResult.Get();
	return shapeResult.Get();
}

JPH::Body *Phys::createBodyFromShape(JPH::RefConst<JPH::Shape> shape, const JPH::Vec3 &translation, const JPH::Quat &rotation) {
	BodyCreationSettings bodySettings(shape, translation, rotation, EMotionType::Dynamic, Layers::MOVING);
	BodyInterface &bodyInterface = getBodyInterface();
	Body* body = bodyInterface.CreateBody(bodySettings);
	bodyInterface.AddBody(body->GetID(), EActivation::DontActivate);
	return body;
}

// doesnt add it to phys system
JPH::Body *Phys::createFakeBodyFromShape(JPH::RefConst<JPH::Shape> shape, const JPH::Vec3 &translation, const JPH::Quat &rotation) {
	BodyCreationSettings bodySettings(shape, translation, rotation, EMotionType::Dynamic, Layers::MOVING);
	BodyInterface &bodyInterface = getBodyInterface();
	Body* body = bodyInterface.CreateBody(bodySettings);
	// bodyInterface.AddBody(body->GetID(), EActivation::DontActivate);
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

// TODO consider just using GetWorldTransform()
glm::mat4 Phys::getBodyTransform(const JPH::Body *body) {
	JPH::RVec3 position = body->GetPosition();
	JPH::Quat rotation = body->GetRotation();

	glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(position.GetX(), position.GetY(), position.GetZ()));
	glm::quat glmQuat(rotation.GetW(), rotation.GetX(), rotation.GetY(), rotation.GetZ());
	glm::mat4 rotationMatrix = glm::toMat4(glmQuat); // TODO use glm::rotate instead???

	return translationMatrix * rotationMatrix;
}

glm::mat4 Phys::getCharacterTransform(const JPH::Ref<JPH::Character> character) {
	JPH::RVec3 position = character->GetPosition();
	JPH::Quat rotation = character->GetRotation();

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

PhysRenderer *Phys::getPhysRenderer() {
	return Phys::phys_renderer.get();
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

void Phys::buildDebugVerts() {
	// TODO call SetCameraPos on the debug renderer
	JPH::BodyManager::DrawSettings settings = JPH::BodyManager::DrawSettings();
	Phys::getPhysSystem()->DrawBodies(settings, Phys::getPhysRenderer());
	// struct DrawSettings
	// {
	// 	bool						mDrawGetSupportFunction = false;				///< Draw the GetSupport() function, used for convex collision detection
	// 	bool						mDrawSupportDirection = false;					///< When drawing the support function, also draw which direction mapped to a specific support point
	// 	bool						mDrawGetSupportingFace = false;					///< Draw the faces that were found colliding during collision detection
	// 	bool						mDrawShape = true;								///< Draw the shapes of all bodies
	// 	bool						mDrawShapeWireframe = false;					///< When mDrawShape is true and this is true, the shapes will be drawn in wireframe instead of solid.
	// 	EShapeColor					mDrawShapeColor = EShapeColor::MotionTypeColor; ///< Coloring scheme to use for shapes
	// 	bool						mDrawBoundingBox = false;						///< Draw a bounding box per body
	// 	bool						mDrawCenterOfMassTransform = false;				///< Draw the center of mass for each body
	// 	bool						mDrawWorldTransform = false;					///< Draw the world transform (which can be different than the center of mass) for each body
	// 	bool						mDrawVelocity = false;							///< Draw the velocity vector for each body
	// 	bool						mDrawMassAndInertia = false;					///< Draw the mass and inertia (as the box equivalent) for each body
	// 	bool						mDrawSleepStats = false;						///< Draw stats regarding the sleeping algorithm of each body
	// 	bool						mDrawSoftBodyVertices = false;					///< Draw the vertices of soft bodies
	// 	bool						mDrawSoftBodyVertexVelocities = false;			///< Draw the velocities of the vertices of soft bodies
	// 	bool						mDrawSoftBodyEdgeConstraints = false;			///< Draw the edge constraints of soft bodies
	// 	bool						mDrawSoftBodyBendConstraints = false;			///< Draw the bend constraints of soft bodies
	// 	bool						mDrawSoftBodyVolumeConstraints = false;			///< Draw the volume constraints of soft bodies
	// 	bool						mDrawSoftBodySkinConstraints = false;			///< Draw the skin constraints of soft bodies
	// 	bool						mDrawSoftBodyLRAConstraints = false;			///< Draw the LRA constraints of soft bodies
	// 	bool						mDrawSoftBodyPredictedBounds = false;			///< Draw the predicted bounds of soft bodies
	// 	ESoftBodyConstraintColor	mDrawSoftBodyConstraintColor = ESoftBodyConstraintColor::ConstraintType; ///< Coloring scheme to use for soft body constraints
	// };
}

void Phys::update(GLfloat deltaTime, int collisionSteps) {
	ZoneScoped;
	Phys::getPhysSystem()->Update(deltaTime, collisionSteps, Phys::getTempAllocator(), Phys::getJobSystem());
}

BodyID Phys::raycastBody(const JPH::Vec3 &origin, const JPH::Vec3 &direction_and_len) {
	
	// printf("ray from %f %f %f, dir %f %f %f\n", origin.GetX(), origin.GetY(), origin.GetZ(), direction_and_len.GetX(), direction_and_len.GetY(), direction_and_len.GetZ());

	// version using broad phase
	// RayCast ray {origin, direction_and_len};
	// ClosestHitCollisionCollector<RayCastBodyCollector> collector;
	// Phys::getBroadPhase().CastRay(ray, collector);
	// return collector.mHit.mBodyID;

	// version using narrow phase
	RRayCast ray {origin, direction_and_len};
	// TODO prevent this object from being constructed here
	RayBroadPhaseFilter filter;
	// res is a simple collector that only registers the first hit
	RayCastResult res; // {<JPH::BroadPhaseCastResult> = {mBodyID = {static cInvalidBodyID = 4294967295, static cBroadPhaseBit = 8388608, static cMaxBodyIndex = 8388607, static cMaxSequenceNumber = 255 '\377', mID = 32512}, mFraction = 1.36915773e+14}, mSubShapeID2 = {static MaxBits = 32, static cEmpty = 4294967295, mValue = 21845}}
	Phys::getNarrowPhase().CastRay(ray, res, filter);
	// if (res.mBodyID.IsInvalid()) {
	// 	printf("no hit\n");
	// } else {
	// 	printf("hit\n");
	// }
	return res.mBodyID;
	

	// can't see SHIT with all these abstractions, here is what gdb says collector.mHit contains:
	// {mBodyID = {static cInvalidBodyID = 4294967295, static cBroadPhaseBit = 8388608, static cMaxBodyIndex = 8388607, static cMaxSequenceNumber = 255 '\377', mID = 1433036460}, mFraction = 3.0611365e-41}
	// BodyID is just a uint32

	// no need for this. if not hit, id == BodyID::cInvalidBodyID
	// if (collector.HadHit()) {
	// 	printf("had a hit. ");
	// } else {
	// 	printf("no hit. ");
	// }
	// printf("id is %u\n", collector.mHit.mBodyID.GetIndexAndSequenceNumber());

}

const BroadPhaseQuery &Phys::getBroadPhase() {
	return Phys::getPhysSystem()->GetBroadPhaseQuery();
}

const NarrowPhaseQuery &Phys::getNarrowPhase() {
	return Phys::getPhysSystem()->GetNarrowPhaseQuery();
}

void Phys::setUserData(JPH::Body *body, UserData data) {
	body->SetUserData(data.data);
}

UserData Phys::getUserData(JPH::Body *body) {
	return UserData(body->GetUserData());
}

UserData Phys::getUserData(JPH::BodyID bodyID) {
	BodyInterface &bodyInterface = getBodyInterface();
	return UserData(bodyInterface.GetUserData(bodyID));
}

// https://jrouwe.github.io/JoltPhysics/class_broad_phase_query.html
// https://jrouwe.github.io/JoltPhysics/class_narrow_phase_query.html
// https://jrouwe.github.io/JoltPhysics/class_shape.html
// TODO clean this up
bool Phys::canBePlaced(const JPH::AABox &aabb, const JPH::Mat44 &transform, JPH::RefConst<JPH::Shape> phys_shape) {
	const JPH::Vec3 scale = JPH::Vec3::sReplicate(1.0f);
	// const JPH::Vec3 pos = JPH::Vec3(static_cast<float>(insertInfo.pos.x), static_cast<float>(insertInfo.pos.y), static_cast<float>(insertInfo.pos.z));
	// const JPH::Quat rot = JPH::Quat(insertInfo.rot.x, insertInfo.rot.y, insertInfo.rot.z, insertInfo.rot.w);
	// const JPH::Mat44 transform = JPH::Mat44::sRotationTranslation(rot, pos);

	// // // since world space bounds is relative to center of mass, I need to apply an offset
	// // JPH::Vec3 centerOfMassOffset = insertInfo.obj->phys_shape->GetCenterOfMass();
	// // JPH::AABox aabb = insertInfo.obj->phys_shape->GetWorldSpaceBounds(JPH::Mat44::sRotationTranslation(rot, pos - centerOfMassOffset), JPH::Vec3::sReplicate(1.0f));
	// JPH::AABox aabb = insertInfo.obj->phys_shape->GetWorldSpaceBounds(transform, scale);


// // Assuming you have a Jolt physics system initialized
// JPH::PhysicsSystem physicsSystem;

// // Create a new body
// JPH::BodyCreationSettings bodySettings(/* parameters */);
// JPH::Body* newBody = physicsSystem.CreateBody(bodySettings);

// // Perform a broad phase query
// JPH::BroadPhaseQuery broadPhaseQuery = physicsSystem.GetBroadPhaseQuery();
// JPH::BroadPhaseLayerFilter layerFilter = /* your layer filter */;
// JPH::ObjectLayerFilter objectFilter = /* your object filter */;
// JPH::BroadPhaseQuery::Results results;
// broadPhaseQuery.CollideAABox(newBody->GetWorldSpaceBounds(), results, layerFilter, objectFilter);

// // Check results for potential collisions
// for (const JPH::BroadPhaseQuery::Result& result : results)
// {
//     JPH::Body* otherBody = physicsSystem.GetBody(result.mBodyID);
//     if (newBody->Collide(otherBody))
//     {
//         // Handle intersection
//         std::cout << "Intersection detected with body ID: " << result.mBodyID << std::endl;
//     }
// }

	// TODO get these out of here
    // Define a collision filter (you can customize this)
    class MyBroadPhaseFilter : public JPH::BroadPhaseLayerFilter {
    public:
        virtual bool ShouldCollide(JPH::BroadPhaseLayer inLayer) const override {
            // You can filter layers if needed, for now return true for all
            return true;
        }
    };

    class MyObjectFilter : public JPH::ObjectLayerFilter {
    public:
        virtual bool ShouldCollide(JPH::ObjectLayer inLayer) const override {
            // Customize this if you need to filter out certain layers
            return true;
        }
    };

	// TODO can I make this stop after first collision?
	class MyBodyCollector : public CollideShapeBodyCollector {
	public:
		// Store body IDs that overlap with the AABB
		// Array<BodyID> overlappingBodies;
		bool collision = false;

		// This function is called for each overlapping body
		virtual void AddHit(const BodyID &inBodyID) override {
			// overlappingBodies.push_back(inBodyID);
			collision = true;
		}
	};

	class MyCollideShapeCollector : public CollideShapeCollector {
	public:
		// This function is called for each hit detected by the collision query
		bool collision = false;
		virtual void AddHit(const CollideShapeResult &inResult) override {
			// Print out information about the hit (BodyID and Fraction)
			// std::cout << "Hit detected! BodyID: " << inResult.mBodyID.GetIndex() << ", Fraction: " << inResult.mFraction << std::endl;

			// You can store results if needed, for example:
			// hits.push_back(inResult);
			collision = true;
		}

		// Store all the hits detected (optional, if you need to process them later)
		// std::vector<CollideShapeResult> hits;
	};

    MyBroadPhaseFilter broadPhaseFilter;
    MyObjectFilter objectFilter;
	MyBodyCollector collector;

    // Perform the broad-phase collision check
	// Jolt/Physics/Collision/Collector/ClosestHitCollisionCollector.h
    Phys::getBroadPhase().CollideAABox(aabb, collector, broadPhaseFilter, objectFilter);

	bool res = true;

	// broad phase collided, so we need to check narrow phase
	if (collector.collision) {
		CollideShapeSettings collideSettings;
		MyCollideShapeCollector collisionCollector;
	// 	void NarrowPhaseQuery::CollideShape 	( 	const Shape *  	inShape,
	// 	Vec3Arg  	inShapeScale,
	// 	RMat44Arg  	inCenterOfMassTransform,
	// 	const CollideShapeSettings &  	inCollideShapeSettings,
	// 	RVec3Arg  	inBaseOffset,
	// 	CollideShapeCollector &  	ioCollector,
	// 	const BroadPhaseLayerFilter &  	inBroadPhaseLayerFilter = { },
	// 	const ObjectLayerFilter &  	inObjectLayerFilter = { },
	// 	const BodyFilter &  	inBodyFilter = { },
	// 	const ShapeFilter &  	inShapeFilter = { } 
	// )
		// TODO add all the filters
		// TODO read the comment and change this, bad fix
		JPH::Vec3 pos = transform.GetTranslation();
		Phys::getNarrowPhase().CollideShape(
			phys_shape,
			scale,
			transform,
			collideSettings,
			pos, // All hit results will be returned relative to this offset, can be zero to get results in world position, but when you're testing far from the origin you get better precision by picking a position that's closer e.g. inCenterOfMassTransform.GetTranslation() since floats are most accurate near the origin 
			collisionCollector
		);

		res = !collisionCollector.collision;
	}

    return res;
}

void Phys::setGravityFactor(const JPH::BodyID bodyID, float gravity) {
	BodyInterface &bodyInterface = getBodyInterface();
	bodyInterface.SetGravityFactor(bodyID, gravity);
}

void Phys::setLayer(const JPH::BodyID bodyID, const JPH::ObjectLayer &layer) {
	BodyInterface &bodyInterface = getBodyInterface();
	bodyInterface.SetObjectLayer(bodyID, layer);
}

JPH::AABox Phys::getAABB(const JPH::Body *body) {
	return body->GetWorldSpaceBounds();
}

// JPH::AABox Phys::getAABB(const JPH::BodyID bodyID) {
// 	BodyInterface &bodyInterface = getBodyInterface();
// 	return bodyInterface.GetWorldSpaceBounds(bodyID);
// }

// JPH::AABox Phys::getAABB(JPH::RefConst<JPH::Character> character) {
// 	return getAABB(character.GetBodyID());
// }

