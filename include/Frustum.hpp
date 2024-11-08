#ifndef FRUSTUM_H
#define FRUSTUM_H

// got lazy, cooked by chatgpt, ignore the comments

#include "types.hpp"
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>
#include "Components.hpp" // for physics component

struct Plane {
    glm::vec3 normal; // Normal vector of the plane
    GLfloat distance;   // Distance from the origin to nearest point in the plane

	Plane() = default;
    constexpr Plane(const glm::vec3& n, GLfloat d) : normal(n), distance(d) {}

	// make a plane from 3 points
	// TODO make this  constexpr
	Plane(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3)
		: normal(glm::normalize(glm::cross(p2 - p1, p3 - p1))), distance(-glm::dot(normal, p1)) {}

    // Compute the signed distance from a point to this plane
    constexpr GLfloat distanceToPoint(const glm::vec3& point) const {
        return glm::dot(normal, point) + distance;
    }

	~Plane() = default;

	bool isOnOrForwardPlane(const glm::vec3 &min, const glm::vec3 &max) const {
		// Determine the positive and negative vertices of the AABB with respect to the plane normal
		glm::vec3 p_vertex = glm::vec3(
			(normal.x >= 0) ? max.x : min.x,
			(normal.y >= 0) ? max.y : min.y,
			(normal.z >= 0) ? max.z : min.z
		);

		// Check if the AABB is on or in front of the plane by testing the "most negative" vertex
		return distanceToPoint(p_vertex) >= 0;
	}

};

struct Frustum {
    Plane planes[6]; // Array of 6 planes

    // // Function to extract planes from a view-projection matrix
    // void extractFromMatrix(const glm::mat4& vpMatrix) {
    //     // Extract planes from the view-projection matrix
    //     planes[0] = Plane(glm::vec3(vpMatrix[0][3] + vpMatrix[0][0], 
    //                                 vpMatrix[1][3] + vpMatrix[1][0], 
    //                                 vpMatrix[2][3] + vpMatrix[2][0]),
    //                       vpMatrix[3][3] + vpMatrix[3][0]); // Left
                          
    //     planes[1] = Plane(glm::vec3(vpMatrix[0][3] - vpMatrix[0][0], 
    //                                 vpMatrix[1][3] - vpMatrix[1][0], 
    //                                 vpMatrix[2][3] - vpMatrix[2][0]),
    //                       vpMatrix[3][3] - vpMatrix[3][0]); // Right
                          
    //     planes[2] = Plane(glm::vec3(vpMatrix[0][3] + vpMatrix[0][1], 
    //                                 vpMatrix[1][3] + vpMatrix[1][1], 
    //                                 vpMatrix[2][3] + vpMatrix[2][1]),
    //                       vpMatrix[3][3] + vpMatrix[3][1]); // Bottom
                          
    //     planes[3] = Plane(glm::vec3(vpMatrix[0][3] - vpMatrix[0][1], 
    //                                 vpMatrix[1][3] - vpMatrix[1][1], 
    //                                 vpMatrix[2][3] - vpMatrix[2][1]),
    //                       vpMatrix[3][3] - vpMatrix[3][1]); // Top
                          
    //     planes[4] = Plane(glm::vec3(vpMatrix[0][3] + vpMatrix[0][2], 
    //                                 vpMatrix[1][3] + vpMatrix[1][2], 
    //                                 vpMatrix[2][3] + vpMatrix[2][2]),
    //                       vpMatrix[3][3] + vpMatrix[3][2]); // Near
                          
    //     planes[5] = Plane(glm::vec3(vpMatrix[0][3] - vpMatrix[0][2], 
    //                                 vpMatrix[1][3] - vpMatrix[1][2], 
    //                                 vpMatrix[2][3] - vpMatrix[2][2]),
    //                       vpMatrix[3][3] - vpMatrix[3][2]); // Far
    // }

	// TODO just copy this from learnopengl
    Frustum(const glm::vec3& position, const glm::vec3& direction, const glm::vec3& up, GLfloat fov, GLfloat aspect, GLfloat nearDist, GLfloat farDist) {
        const glm::vec3 right = glm::normalize(glm::cross(direction, up));
        const glm::vec3 upVector = glm::normalize(glm::cross(right, direction));
        const glm::vec3 forward = glm::normalize(direction);

		// change this tan to glm?
        const GLfloat nearHeight = 2.0f * tanf(glm::radians(fov) / 2.0f) * nearDist;
        const GLfloat nearWidth = nearHeight * aspect;
        const GLfloat farHeight = 2.0f * tanf(glm::radians(fov) / 2.0f) * farDist;
        const GLfloat farWidth = farHeight * aspect;

        const glm::vec3 nearCenter = position + forward * nearDist;
        const glm::vec3 farCenter = position + forward * farDist;

        const glm::vec3 nearTopLeft = nearCenter + (upVector * (nearHeight / 2.0f)) - (right * (nearWidth / 2.0f));
        const glm::vec3 nearTopRight = nearCenter + (upVector * (nearHeight / 2.0f)) + (right * (nearWidth / 2.0f));
        const glm::vec3 nearBottomLeft = nearCenter - (upVector * (nearHeight / 2.0f)) - (right * (nearWidth / 2.0f));
        const glm::vec3 nearBottomRight = nearCenter - (upVector * (nearHeight / 2.0f)) + (right * (nearWidth / 2.0f));

        const glm::vec3 farTopLeft = farCenter + (upVector * (farHeight / 2.0f)) - (right * (farWidth / 2.0f));
        const glm::vec3 farTopRight = farCenter + (upVector * (farHeight / 2.0f)) + (right * (farWidth / 2.0f));
        const glm::vec3 farBottomLeft = farCenter - (upVector * (farHeight / 2.0f)) - (right * (farWidth / 2.0f));
        const glm::vec3 farBottomRight = farCenter - (upVector * (farHeight / 2.0f)) + (right * (farWidth / 2.0f));

        planes[0] = Plane(nearTopRight, nearTopLeft, farTopLeft); // Top
        planes[1] = Plane(nearBottomLeft, nearBottomRight, farBottomRight); // Bottom
        planes[2] = Plane(nearTopLeft, nearBottomLeft, farBottomLeft); // Left
        planes[3] = Plane(nearBottomRight, nearTopRight, farBottomRight); // Right
        planes[4] = Plane(nearTopLeft, nearTopRight, nearBottomRight); // Near
        planes[5] = Plane(farTopRight, farTopLeft, farBottomLeft); // Far
    }

	~Frustum() = default;

	bool contains(const Physics &phys) const {
		JPH::AABox aabb = phys.getAABB();

		// TODO ugly, look into Vec3 type??
		glm::vec3 max(aabb.mMax.GetX(), aabb.mMax.GetY(), aabb.mMax.GetZ());
		glm::vec3 min(aabb.mMin.GetX(), aabb.mMin.GetY(), aabb.mMin.GetZ());


		return planes[0].isOnOrForwardPlane(min, max) && 
			   planes[1].isOnOrForwardPlane(min, max) &&
			   planes[2].isOnOrForwardPlane(min, max) &&
			   planes[3].isOnOrForwardPlane(min, max) &&
			   planes[4].isOnOrForwardPlane(min, max) &&
			   planes[5].isOnOrForwardPlane(min, max);


		// for (const Plane &plane : planes) {


			// // only needed to check if fully inside frustum???
			// // glm::vec3 positive_vertex(
			// // 	plane.normal.x >= 0 ? max.x : min.x,
			// // 	plane.normal.y >= 0 ? max.y : min.y,
			// // 	plane.normal.z >= 0 ? max.z : min.z
			// // );
			// glm::vec3 negative_vertex(
			// 	plane.normal.x >= 0 ? min.x : max.x,
			// 	plane.normal.y >= 0 ? min.y : max.y,
			// 	plane.normal.z >= 0 ? min.z : max.z
			// );

			// // check if negative vertex is outside the plane
			// // TODO cursed, just multiply the vectors??
			// if (plane.normal.x * negative_vertex.x + plane.normal.y * negative_vertex.y + plane.normal.z * negative_vertex.z + plane.distance > 0) {
			// 	return true;
			// }
		// }
	}
};

#endif
