#version 460 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;


out VS_OUT {
	vec2 v_TexCoord;
	flat int v_MaterialID; // flat since it is always the same between all vertices. int because reasons, should be an uint
	flat vec3 v_Normal;    // in view space
	vec3 v_FragPos;        // in view space
} vs_out;

// uniform mat3 u_NormalMatrix; // since it is constant every single vertex

void main()
{
	vec4 viewspace_position = u_View * u_Model * vec4(aPos, 1.0);
	vs_out.v_FragPos = vec3(viewspace_position);

	mat3 normalMatrix = mat3(transpose(inverse(u_View * u_Model)));
	vs_out.v_Normal = normalMatrix * aNormal;
	vs_out.v_MaterialID = int(1);
	vs_out.v_TexCoord = aTexCoord;

	gl_Position = u_Projection * viewspace_position;
}
