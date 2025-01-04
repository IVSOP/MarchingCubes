#version 460 core

// per vertex
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

// uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;

uniform samplerBuffer u_TransformTBO;
#define VEC4_IN_TRANSFORM 4


out VS_OUT {
	vec2 v_TexCoord;
	flat int v_MaterialID; // flat since it is always the same between all vertices. int because reasons, should be an uint
	flat vec3 v_Normal;    // in view space
	vec3 v_FragPos;        // in view space
} vs_out;

// uniform mat3 u_NormalMatrix; // since it is constant every single vertex

void main()
{
	mat4 model;
	model[0] = texelFetch(u_TransformTBO, 0 + (gl_InstanceID * VEC4_IN_TRANSFORM));
	model[1] = texelFetch(u_TransformTBO, 1 + (gl_InstanceID * VEC4_IN_TRANSFORM));
	model[2] = texelFetch(u_TransformTBO, 2 + (gl_InstanceID * VEC4_IN_TRANSFORM));
	model[3] = texelFetch(u_TransformTBO, 3 + (gl_InstanceID * VEC4_IN_TRANSFORM));

	vec4 viewspace_position = u_View * model * vec4(aPos, 1.0);
	vs_out.v_FragPos = vec3(viewspace_position);

	// TODO test getting the normal matrix from the CPU
	mat3 normalMatrix = mat3(transpose(inverse(u_View * model)));
	vs_out.v_Normal = normalMatrix * aNormal;
	vs_out.v_MaterialID = int(1);
	vs_out.v_TexCoord = aTexCoord;

	gl_Position = u_Projection * viewspace_position;
}
