#version 460 core

layout(location = 0) out vec4 color;
layout(location = 1) out vec4 brightColor;

in vec3 aTexCoord;

uniform samplerCube skybox;

void main()
{    
    color = texture(skybox, aTexCoord);
	brightColor = vec4(0.0, 0.0, 0.0, 1.0);
}
