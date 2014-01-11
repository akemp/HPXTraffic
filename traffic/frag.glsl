#version 330

uniform sampler2D tex;
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
        col = texture2D(tex, tex_coords_frag);
	    color = col*(1.0-dist)+vec4(0.95)*dist;
}
