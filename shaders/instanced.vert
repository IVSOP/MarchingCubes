#version 410 core

// per vertex
layout (location = 0) in int aVertID;

// per instance
layout (location = 1) in ivec3 aLocalPos;
layout (location = 2) in ivec3 aEdgeIDs;
layout (location = 3) in int aMaterialID;

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;

const vec3 lookup_array[] = vec3[12](
	vec3(0.5, 0.0, 0.0),
	vec3(1.0, 0.0, 0.5),
	vec3(0.5, 0.0, 1.0),
	vec3(0.0, 0.0, 0.5),
	vec3(0.5, 1.0, 0.0),
	vec3(1.0, 1.0, 0.5),
	vec3(0.5, 1.0, 1.0),
	vec3(0.0, 1.0, 0.5),
	vec3(0.0, 0.5, 0.0),
	vec3(1.0, 0.5, 0.0),
	vec3(1.0, 0.5, 1.0),
	vec3(0.0, 0.5, 1.0)
);

void main()
{
	// using vert id, get edge id
	// using edge id, get the position inside voxel
	// then, add position inside chunk
	vec3 position = lookup_array[aEdgeIDs[aVertID]] + aLocalPos;

	gl_Position = u_Projection * u_View * u_Model * vec4(position, 1.0);
}
