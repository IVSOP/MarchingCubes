#version 460 core

// per vertex
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

uniform mat4 u_View;
uniform mat4 u_Projection;

uniform samplerBuffer u_TransformTBO;
#define VEC4_IN_TRANSFORM 4

out VS_OUT {
	vec3 v_Normal;
} vs_out;

void main()
{
	mat4 model;
	model[0] = texelFetch(u_TransformTBO, 0 + (gl_InstanceID * VEC4_IN_TRANSFORM));
	model[1] = texelFetch(u_TransformTBO, 1 + (gl_InstanceID * VEC4_IN_TRANSFORM));
	model[2] = texelFetch(u_TransformTBO, 2 + (gl_InstanceID * VEC4_IN_TRANSFORM));
	model[3] = texelFetch(u_TransformTBO, 3 + (gl_InstanceID * VEC4_IN_TRANSFORM));

	vec4 viewspace_position = u_View * model * vec4(aPos, 1.0);

	// TODO test getting the normal matrix from the CPU
	mat3 normalMatrix = mat3(transpose(inverse(u_View * model)));
	vs_out.v_Normal = normalMatrix * aNormal;

	gl_Position = viewspace_position;
}
