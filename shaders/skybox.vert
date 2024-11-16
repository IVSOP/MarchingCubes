#version 460 core

layout (location = 0) in vec3 aPos;

out vec3 aTexCoord;

uniform mat4 u_View;
uniform mat4 u_Projection;

void main()
{
    aTexCoord = aPos;
    vec4 pos = u_Projection * u_View * vec4(aPos, 1.0);
	gl_Position = pos.xyww; // make it so that z = w, meaning the depth value is always going to be 1 (max depth) (what the fuck)
}  
