#version 330

uniform sampler2D tex;
uniform sampler2D roadMap;
uniform mat4 model;
uniform mat4 projection;

in vec2 tex_coords_frag;
in vec3 pos;
in float texInd_frag;
out vec4 color;
void main()
{

    float dist = distance(model[3], projection * vec4(pos,1))/100.0;
    if (dist > 1)
        discard;
    vec4 col = vec4(0);
        float red = texture2D(roadMap, tex_coords_frag/512.0).r;
        col = texture2D(tex, pos.xz/10.0)*red*2.0;
	    color = col*(1.0-dist)+vec4(0.95)*dist;
}
