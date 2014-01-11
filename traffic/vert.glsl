#version 330
#define cars 512

uniform mat4 projection;
uniform mat4 modelview;
uniform mat4 model;

layout(location = 0) in vec3 vert1;
layout(location = 1) in vec3 normal_modelspace;
layout(location = 2) in vec2 tex_coords;
layout(location = 3) in float texInd;


out vec3 pos;
out vec2 tex_coords_frag;
out float texInd_frag;

void main()
{
    float ind = texInd;
	vec4 position = vec4(vert1, 1.0);
    pos = position.xyz;

	tex_coords_frag = tex_coords;	

    texInd_frag = ind;

	gl_Position = projection * position;
}
