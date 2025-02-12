#ifndef PHYS_HPP
#define PHYS_HPP

#include "types.hpp"
#include "stdlib.hpp"
#include "Json.hpp"
#include "CustomVec.hpp"

#include <Jolt/Jolt.h>
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>
#include <Jolt/Physics/PhysicsScene.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/CylinderShape.h>
#include <Jolt/Physics/Collision/Shape/RotatedTranslatedShape.h>
#include <Jolt/Physics/Collision/Shape/ConvexHullShape.h>
#include <Jolt/Physics/Collision/Shape/MeshShape.h>
#include <Jolt/Physics/Collision/Shape/StaticCompoundShape.h>
#include <Jolt/Physics/Character/Character.h>
#include <Jolt/Physics/Character/CharacterVirtual.h>
#include <Jolt/Physics/Collision/RayCast.h>
#include <Jolt/Physics/Collision/CastResult.h>
#include <Jolt/Physics/Collision/CollideShape.h>
#include <Jolt/Physics/Collision/CollisionCollectorImpl.h>

#include "PhysRenderer.hpp"

#include "ObjectInfo.hpp"

JPH_SUPPRESS_WARNINGS

// TODO GET THIS THE FUCK OUT OF HERE
struct InsertInfo;

// JPH_NAMESPACE_BEGIN

// Callback for traces, connect this to your own trace function if you have one
void TraceImpl(const char *inFMT, ...);

#ifdef JPH_ENABLE_ASSERTS

	// Callback for asserts, connect this to your own assert handler if you have one
	static bool AssertFailedImpl(const char *inExpression, const char *inMessage, const char *inFile, JPH::uint inLine);

#endif // JPH_ENABLE_ASSERTS

namespace Layers
{
	static constexpr JPH::ObjectLayer NON_MOVING = 0;
	static constexpr JPH::ObjectLayer MOVING = 1;
	static constexpr JPH::ObjectLayer NOCLIP = 2;
	static constexpr JPH::ObjectLayer NUM_LAYERS = 3;
};

/// Class that determines if two object layers can collide
class ObjectLayerPairFilterImpl : public JPH::ObjectLayerPairFilter
{
public:
	virtual bool ShouldCollide(JPH::ObjectLayer inObject1, JPH::ObjectLayer inObject2) const override
	{
		switch (inObject1)
		{
		case Layers::NON_MOVING:
			return inObject2 == Layers::MOVING; // Non moving only collides with moving
		case Layers::MOVING:
			// return true;
			// Moving collides with everything, except for noclip
			return inObject2 != Layers::NOCLIP;
		case Layers::NOCLIP:
			return false; // noclip never collides with anything
		default:
			JPH_ASSERT(false);
			return false;
		}
	}
};

// Each broadphase layer results in a separate bounding volume tree in the broad phase. You at least want to have
// a layer for non-moving and moving objects to avoid having to update a tree full of static objects every frame.
// You can have a 1-on-1 mapping between object layers and broadphase layers (like in this case) but if you have
// many object layers you'll be creating many broad phase trees, which is not efficient. If you want to fine tune
// your broadphase layers define JPH_TRACK_BROADPHASE_STATS and look at the stats reported on the TTY.
namespace BroadPhaseLayers
{
	static constexpr JPH::BroadPhaseLayer NON_MOVING(0);
	static constexpr JPH::BroadPhaseLayer MOVING(1);
	static constexpr JPH::uint NUM_LAYERS(2);
};

// BroadPhaseLayerInterface implementation
// This defines a mapping between object and broadphase layers.
class BPLayerInterfaceImpl final : public JPH::BroadPhaseLayerInterface
{
public:
	BPLayerInterfaceImpl()
	{
		// Create a mapping table from object to broad phase layer
		mObjectToBroadPhase[Layers::NON_MOVING] = BroadPhaseLayers::NON_MOVING;
		mObjectToBroadPhase[Layers::MOVING] = BroadPhaseLayers::MOVING;
		mObjectToBroadPhase[Layers::NOCLIP] = BroadPhaseLayers::MOVING; // TODO I don't know what to do here but it prob doesn't matter too much
	}

	virtual JPH::uint GetNumBroadPhaseLayers() const override
	{
		return BroadPhaseLayers::NUM_LAYERS;
	}

	virtual JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer inLayer) const override
	{
		JPH_ASSERT(inLayer < Layers::NUM_LAYERS);
		return mObjectToBroadPhase[inLayer];
	}

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
	virtual const char *GetBroadPhaseLayerName(JPH::BroadPhaseLayer inLayer) const override
	{
		switch ((JPH::BroadPhaseLayer::Type)inLayer)
		{
		case (JPH::BroadPhaseLayer::Type)BroadPhaseLayers::NON_MOVING:	return "NON_MOVING";
		case (JPH::BroadPhaseLayer::Type)BroadPhaseLayers::MOVING:		return "MOVING";
		default:													JPH_ASSERT(false); return "INVALID";
		}
	}
#endif // JPH_EXTERNAL_PROFILE || JPH_PROFILE_ENABLED

private:
	JPH::BroadPhaseLayer mObjectToBroadPhase[Layers::NUM_LAYERS];
};

/// Class that determines if an object layer can collide with a broadphase layer
class ObjectVsBroadPhaseLayerFilterImpl : public JPH::ObjectVsBroadPhaseLayerFilter
{
public:
	virtual bool ShouldCollide(JPH::ObjectLayer inLayer1, JPH::BroadPhaseLayer inLayer2) const override
	{
		switch (inLayer1)
		{
		case Layers::NON_MOVING:
			return inLayer2 == BroadPhaseLayers::MOVING;
		case Layers::MOVING:
			return true;
		case Layers::NOCLIP:
			return false; // is this right???
		default:
			JPH_ASSERT(false);
			return false;
		}
	}
};

// Custom BroadPhaseLayerFilter that excludes non moving
class RayBroadPhaseFilter : public JPH::BroadPhaseLayerFilter
{
public:
    JPH::BroadPhaseLayer excluded_layer = BroadPhaseLayers::NON_MOVING;

    RayBroadPhaseFilter() = default;

    virtual bool ShouldCollide(JPH::BroadPhaseLayer layer) const override
    {
        return layer != excluded_layer;  // Exclude the specific layer
    }
};


#include <entt.hpp>
// must fit into uint64, or be a pointer. if pointer, do
// static_assert(sizeof(uint64) == sizeof(void *), "Cannot fit a pointer into Body user data");
// TODO improve this so it's a template
struct UserData {
	constexpr UserData(JPH::uint64 data) : data(data) {} // compiler should optimize this heavily

	constexpr UserData(entt::entity entity) : data(static_cast<JPH::uint64>(entity)) {}

	~UserData() = default;

	constexpr entt::entity getEntity() const {
		return static_cast<entt::entity>(data);
	}

	JPH::uint64 data;
};

class Phys {
private:
	static std::unique_ptr<JPH::PhysicsSystem> phys_system;
	static std::unique_ptr<JPH::TempAllocatorImpl> temp_allocator;
	static std::unique_ptr<JPH::JobSystem> job_system;
	static std::unique_ptr<PhysRenderer> phys_renderer;

public:
	static void setup_phys();
	static PhysRenderer *getPhysRenderer();
	static void buildDebugVerts();

	static JPH::BodyInterface &getBodyInterface();

	// specific for terrain since it might have specific options (like not being kinematic) 
	static JPH::Body *createTerrain(const JPH::TriangleList &triangles);
	static JPH::Body *createTerrain(const JPH::TriangleList &triangles, const glm::vec3 &coords);
	// DO NOT USE
	static JPH::Body *createTerrainWithNormals(const JPH::TriangleList &triangles, const glm::vec3 &coords, const JPH::Vec3 *normals);

	static JPH::Body *createBody(const JPH::TriangleList &triangles);
	static JPH::Body *createBody(const JPH::TriangleList &triangles, const glm::vec3 &coords);
	// read the shapes from the json and make them into a compound shape, then create a body with it
	static JPH::RefConst<JPH::Shape> createShapeFromJson(const json &data);
	static JPH::RefConst<JPH::Shape> createConvexHull(const CustomVec<ModelVertex> &verts, const std::vector<GLuint> &indices);
	static JPH::Body *createBodyFromShape(JPH::RefConst<JPH::Shape> shape, const JPH::Vec3 &translation, const JPH::Quat &rotation);
	static JPH::Body *createFakeBodyFromShape(JPH::RefConst<JPH::Shape> shape, const JPH::Vec3 &translation, const JPH::Quat &rotation);
	static void activateBody(const JPH::Body *body);
	static void addBodyToSystem(const JPH::Body *body);

	static glm::mat4 getBodyTransform(const JPH::Body *body);
	static glm::mat4 getCharacterTransform(const JPH::Ref<JPH::Character> character);

	static void destroyBody(JPH::Body *body);
	static void setBodyMeshShape(JPH::Body *body, const JPH::TriangleList &triangles);

	static JPH::PhysicsSystem *getPhysSystem();
	static JPH::TempAllocatorImpl *getTempAllocator();
	static JPH::JobSystem *getJobSystem();

	static void update(GLfloat deltaTime, int collisionSteps);

	// casts a ray and returns first body it hits (or null)
	static JPH::BodyID raycastBody(const JPH::Vec3 &origin, const JPH::Vec3 &direction_and_len);
	
	static const JPH::BroadPhaseQuery &getBroadPhase();
	static const JPH::NarrowPhaseQuery &getNarrowPhase();

	static void setUserData(JPH::Body *body, UserData data);
	static UserData getUserData(JPH::Body *body);
	static UserData getUserData(JPH::BodyID bodyID);

	static bool canBePlaced(const JPH::AABox &aabb, const JPH::Mat44 &transform, JPH::RefConst<JPH::Shape> phys_shape);

	static void setGravityFactor(const JPH::BodyID bodyID, float gravity);
	static void setLayer(const JPH::BodyID bodyID, const JPH::ObjectLayer &layer);

	static JPH::AABox getAABB(const JPH::Body *body);
	// static JPH::AABox getAABB(const JPH::BodyID bodyID);
	// static JPH::AABox getAABB(JPH::RefConst<JPH::Character> character);
};

// JPH_NAMESPACE_END

#endif
