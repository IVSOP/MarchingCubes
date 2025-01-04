#version 410 core

// per vertex
layout (location = 0) in int aVertID;

// per instance
layout (location = 1) in int aData;

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
	// if using 460 these can all be const, not with 410
	uint local_pos_x = (aData >> 27) & 0x0000001F;
	uint local_pos_y = (aData >> 22) & 0x0000001F;
	uint local_pos_z = (aData >> 17) & 0x0000001F;

	// I will assume the compiler optimizes this, otherwise try placing all values into uvec3 or something
	uint edgeID;
	switch(aVertID) {
		case 0:
			edgeID = (aData >> 13) & 0x0000000F;
			break;
		case 1:
			edgeID = (aData >> 9) & 0x0000000F;
			break;
		case 2:
			edgeID = (aData >> 5) & 0x0000000F;
			break;
	}

	uint materialID  =  aData & 0x0000001F;

	vec3 position = lookup_array[edgeID] + vec3(local_pos_x, local_pos_y, local_pos_z);

	gl_Position = u_Projection * u_View * u_Model * vec4(position, 1.0);
}
