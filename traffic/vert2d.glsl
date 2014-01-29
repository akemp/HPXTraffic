#version 330
#define cars 512

uniform mat4 projection;
uniform mat4 modelview;
uniform mat4 model;

layout(location = 0) in vec3 vert1;
layout(location = 1) in vec3 normal_modelspace;
layout(location = 2) in vec2 tex_coords;
layout(location = 3) in float texInd;


out vec2 tex_coords_frag;

void main()
{
    // convert the rectangle from pixels to 0.0 to 1.0
    vec2 zeroToOne = vert1.xz;

    // convert from 0->1 to 0->2
    vec2 zeroToTwo = zeroToOne * 2.0;

    // convert from 0->2 to -1->+1 (clipspace)
    vec2 clipSpace = zeroToTwo - vec2(1.0);
	tex_coords_frag = vert1.xz;	
    gl_Position = vec4(clipSpace,0.0,1.0);

}
