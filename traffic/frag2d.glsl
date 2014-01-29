#version 330

uniform sampler2D tex;
uniform mat4 model;
uniform mat4 modelView;
uniform mat4 projection;

in vec2 tex_coords_frag;
in vec3 pos;
in float texInd_frag;
out vec4 color;
void main()
{
	color = texture2D(tex, tex_coords_frag);
}
