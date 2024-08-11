#include "PhysRenderer.hpp"
#include "Logs.hpp"

void PhysRenderer::DrawLine(JPH::RVec3Arg inFrom, JPH::RVec3Arg inTo, JPH::ColorArg inColor) {
	print_error("Not implemented!");
}

void PhysRenderer::DrawTriangle(JPH::RVec3Arg inV1, JPH::RVec3Arg inV2, JPH::RVec3Arg inV3, JPH::ColorArg inColor, ECastShadow inCastShadow) {
	this->verts.emplace_back(glm::vec3(inV1.GetX(), inV1.GetY(), inV1.GetZ()), glm::vec4(inColor.r, inColor.g, inColor.b, inColor.a));
	this->verts.emplace_back(glm::vec3(glm::vec3(inV2.GetX(), inV2.GetY(), inV2.GetZ())), glm::vec4(inColor.r, inColor.g, inColor.b, inColor.a));
	this->verts.emplace_back(glm::vec3(glm::vec3(inV3.GetX(), inV3.GetY(), inV3.GetZ())), glm::vec4(inColor.r, inColor.g, inColor.b, inColor.a));
}

void PhysRenderer::DrawText3D(JPH::RVec3Arg inPosition, const std::string_view &inString, JPH::ColorArg inColor, float inHeight) {
	print_error("Not implemented!");
}

void PhysRenderer::clearVerts() {
	this->verts.clear();
}

const std::vector<PhysVertex> &PhysRenderer::getVerts() const {
	return this->verts;
}
