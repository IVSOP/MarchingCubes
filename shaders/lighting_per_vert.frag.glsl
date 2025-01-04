#version 460 core

layout(location = 0) out vec4 color;
layout(location = 1) out vec4 brightColor;

in VS_OUT {
	vec2 v_TexCoord;
	flat int texture_id; // flat since it is always the same between all vertices. int because reasons, should be an uint
	flat vec3 color;
} fs_in;

uniform sampler2DArray u_TextureArraySlot;
uniform mat4 u_View; // view from the MVP

uniform float u_BloomThreshold = 1.0;

void main() {
	// apply texture at the end, merge colors
	color = vec4(fs_in.color, 1.0) * texture(u_TextureArraySlot, vec3(fs_in.v_TexCoord.xy, fs_in.texture_id));

	float brightness = dot(color.rgb, vec3(0.2126, 0.7152, 0.0722)); // common approximation of luminance based on human perception of color (or so I'm told)
    if(brightness > u_BloomThreshold) {
        brightColor = vec4(color.rgb, 1.0);
	} else {
        brightColor = vec4(0.0, 0.0, 0.0, 1.0);
	}
}
