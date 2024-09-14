#version 460 core

layout(location = 0) out vec4 color;
layout(location = 1) out vec4 brightColor;


in GS_OUT {
	vec4 g_Color;
} fs_in;

uniform float u_BloomThreshold = 1.0;

void main() {

	color = fs_in.g_Color;
	float brightness = dot(color.rgb, vec3(0.2126, 0.7152, 0.0722)); // common approximation of luminance based on human perception of color (or so I'm told)
    if(brightness > u_BloomThreshold) {
        brightColor = vec4(color.rgb, 1.0);
	} else {
        brightColor = vec4(0.0, 0.0, 0.0, 1.0);
	}
}
