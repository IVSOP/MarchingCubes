#ifndef PHYSRENDERER_H
#define PHYSRENDERER_H

// was having include errors I cant handle this anymore fuck it
#include "types.hpp"
#include "stdlib.hpp"
#include "Json.hpp"
#include "Vertex.hpp"

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
#include <Jolt/Physics/Collision/Shape/MeshShape.h>
#include <Jolt/Physics/Collision/Shape/StaticCompoundShape.h>

// TODO disable debug rendering on final build
// #ifndef JPH_DEBUG_RENDERER
// #define JPH_DEBUG_RENDERER
// #endif
#include <Jolt/Renderer/DebugRendererSimple.h>

JPH_SUPPRESS_WARNINGS

// TODO fully implement DebugRenderer

// use
// physicsSystem.DrawConstraints();
// physicsSystem.DrawBodies();
// to make this class be called

class PhysRenderer : public JPH::DebugRendererSimple {
public:
	PhysRenderer() = default;
	~PhysRenderer() = default;

	virtual void DrawLine(JPH::RVec3Arg inFrom, JPH::RVec3Arg inTo, JPH::ColorArg inColor) override;
	// instead of drawing, just save them to be batch drawn later
	virtual void DrawTriangle(JPH::RVec3Arg inV1, JPH::RVec3Arg inV2, JPH::RVec3Arg inV3, JPH::ColorArg inColor, ECastShadow inCastShadow) override;
	virtual void DrawText3D(JPH::RVec3Arg inPosition, const std::string_view &inString, JPH::ColorArg inColor, float inHeight) override;

	void clearVerts();
	const std::vector<PhysVertex> &getVerts() const;

private:
	std::vector<PhysVertex> verts;
};

#endif
