#version 460 core

// per vertex
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;

uniform samplerBuffer u_TransformTBO;
#define VEC4_IN_TRANSFORM 4

void main()
{
	// const float scale = 1.1;
	// const vec3 translation = vec3(-1.0f, 0.0f, -1.0f);
	// const mat4 scaleMatrix = mat4(
	// 	scale,  0.0,   0.0,  0.0,
	// 	 0.0,  scale,  0.0,  0.0,
	// 	 0.0,   0.0,  scale, 0.0,
	// 	translation.x, translation.y, translation.z,  1.0
	// );


	mat4 model;
	model[0] = texelFetch(u_TransformTBO, 0 + (gl_InstanceID * VEC4_IN_TRANSFORM));
	model[1] = texelFetch(u_TransformTBO, 1 + (gl_InstanceID * VEC4_IN_TRANSFORM));
	model[2] = texelFetch(u_TransformTBO, 2 + (gl_InstanceID * VEC4_IN_TRANSFORM));
	model[3] = texelFetch(u_TransformTBO, 3 + (gl_InstanceID * VEC4_IN_TRANSFORM));

	model *= u_Model;

	vec4 viewspace_position = u_View * model * vec4(aPos, 1.0);
	gl_Position = u_Projection * viewspace_position;
}
