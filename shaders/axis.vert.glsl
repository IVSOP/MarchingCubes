#version 410 core

// per vertex
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec4 aColor;

out vec4 v_Color;

uniform mat4 u_MVP;

void main()
{
	v_Color = aColor;
	gl_Position = u_MVP * vec4(aPos, 1.0f);
}
