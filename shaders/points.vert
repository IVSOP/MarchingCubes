#version 410 core

// per vertex
layout (location = 0) in vec3 aPos;

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;

uniform samplerBuffer u_ChunkInfoTBO;
#define VEC4_IN_CHUNKINFO 1

void main()
{
	gl_PointSize = 5.0f;
	gl_Position = u_Projection * u_View * u_Model * vec4(aPos, 1.0);
}
