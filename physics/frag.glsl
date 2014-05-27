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
    
    float dist = distance(modelView[3], projection * vec4(pos,1))/100.0;
    if (dist > 1)
        discard;
    vec4 col = vec4(0);
        col = texture2D(tex, tex_coords_frag);
        col.rgb /= (texInd_frag*0.25);
	    color = vec4(col.rgb*(1.0-dist)+vec3(0.95)*dist,1.0);
}
