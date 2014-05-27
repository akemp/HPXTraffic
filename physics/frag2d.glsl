#version 330

uniform sampler2D tex;

in vec2 tex_coords_frag;
in vec3 pos;
in float texInd_frag;
out vec4 color;
void main()
{
	vec2 ratio = vec2(512.0,512.0)/vec2(64.0,64.0);
	color = texture2D(tex, tex_coords_frag);
}
