#include "embree.hpp"

#define EPSILON 1e-5f

// scene is a pointer, I hate when they do things this way
// direction can be any, I just picked +x. however, all the code assumes this is the case, you can't just change the direction without breaking everything
uint32_t intersect(const glm::vec3 &origin, float tfar, RTCScene scene) {
	// Ray structure
	RTCRayHit rayHit;
	rayHit.ray.org_x = origin.x;
	rayHit.ray.org_y = origin.y;
	rayHit.ray.org_z = origin.z;

	rayHit.ray.dir_x = 1.0f;
	rayHit.ray.dir_y = 0.0f;
	rayHit.ray.dir_z = 0.0f;

	rayHit.ray.tnear = 0.0f;
	// rayHit.ray.tfar = std::numeric_limits<float>::infinity();

	rayHit.ray.time = 0.0f;    // Time of the ray for motion blur
	rayHit.ray.mask = -1;      // Mask to select geometry
	rayHit.ray.id = 0;         // Ray ID
	rayHit.ray.flags = 0;      // Flags


	// // 0 != RTC_INVALID_GEOMETRY_ID
	// rayHit.hit.geomID = 0;

	uint32_t intersections = 0;

	// don't like breaking in loops but whatever got lazy
	while (true) {
		// reset hit struct
		rayHit.hit.geomID = RTC_INVALID_GEOMETRY_ID;
		rayHit.hit.primID = RTC_INVALID_GEOMETRY_ID;
		rayHit.hit.instID[0] = RTC_INVALID_GEOMETRY_ID;

		// reset ray far distance
		rayHit.ray.tfar = tfar;


		// printf("Tracing ray starting at %f %f %f direction %f %f %f near %f far %f\n", rayHit.ray.org_x, rayHit.ray.org_y, rayHit.ray.org_z,
		// 	rayHit.ray.dir_x, rayHit.ray.dir_y, rayHit.ray.dir_z, rayHit.ray.tnear, rayHit.ray.tfar);
		rtcIntersect1(scene, &rayHit);
		if (rayHit.hit.geomID != RTC_INVALID_GEOMETRY_ID) {
			// printf("Intersection at t = %f. this means point %f %f %f\n", rayHit.ray.tfar, rayHit.ray.org_x + rayHit.ray.tfar, rayHit.ray.org_y, rayHit.ray.org_z);
			intersections++;
			// move the ray forward a bit
			rayHit.ray.org_x += rayHit.ray.tfar + EPSILON;

			// decrease its max distance since I have now moved it forward
			// .............. TODO, for now I just reset it
			rayHit.ray.tfar = tfar;
		} else {
			break;
		}
	}

	// rayHit.ray.org_x += rayHit.ray.dir_x * (rayHit.ray.tfar + epsilon);
	// rayHit.ray.org_y += rayHit.ray.dir_y * (rayHit.ray.tfar + epsilon);
	// rayHit.ray.org_z += rayHit.ray.dir_z * (rayHit.ray.tfar + epsilon);

	return intersections;
}

// possible optimization: since I trace in +x, I could do an entire line of points all at once, reusing work
void EmbreeWrapper::marcheCubes(MarchingCubesObject *obj, const CustomVec<ModelVertex> &verts, const std::vector<GLuint> &indices, uint32_t len_x, uint32_t len_y, uint32_t len_z) {
	// 1. Initialize Embree device
	RTCDevice device = rtcNewDevice(nullptr);
	CRASH_IF(!device, "Failed to create Embree device");
	RTCScene scene = rtcNewScene(device);

	const glm::vec3 offset = glm::vec3(static_cast<GLfloat>(len_x) / 4.0f, static_cast<GLfloat>(len_y) / 4.0f, static_cast<GLfloat>(len_z) / 4.0f);

	CustomVec<glm::vec3> vertices = CustomVec<glm::vec3>(verts.size() * 3);
	for (unsigned int i = 0; i < verts.size(); i++) {
		const ModelVertex& vert = verts[i];
		// the xyz provided are total len of the final grid
		// it is expected that the mesh is centered around (0,0,0)
		// for simplicity, move the entire mesh itself
		vertices.emplace_back(vert.coords + offset);
	}

	// Create a new triangle mesh geometry
	RTCGeometry mesh = rtcNewGeometry(device, RTC_GEOMETRY_TYPE_TRIANGLE);

	// Set vertex buffer
	float* vertexBuffer = (float*)rtcSetNewGeometryBuffer(mesh, RTC_BUFFER_TYPE_VERTEX, 0, RTC_FORMAT_FLOAT3, sizeof(glm::vec3), vertices.size());
	std::memcpy(vertexBuffer, vertices.data(), sizeof(glm::vec3) * vertices.size());

	// Set index buffer
	// why 3 * sizeof()?????? makes no sense but whatever
	unsigned int* indexBuffer = (unsigned int*)rtcSetNewGeometryBuffer(mesh, RTC_BUFFER_TYPE_INDEX, 0, RTC_FORMAT_UINT3, 3 * sizeof(unsigned int), indices.size() / 3);
	std::memcpy(indexBuffer, indices.data(), sizeof(unsigned int) * indices.size());

	// Commit geometry changes
	rtcCommitGeometry(mesh);
	rtcAttachGeometry(scene, mesh);
	rtcReleaseGeometry(mesh);  // Safe to release after attachment

	// Commit scene changes
	rtcCommitScene(scene);

	// RTCRayQueryContext context;
	// rtcInitRayQueryContext(&context);

	intersect(glm::vec3(0.0f, 5.0f, 5.0f), len_x + 1.0f, scene);

	rtcReleaseScene(scene);
	rtcReleaseDevice(device);

	exit(1);
}
