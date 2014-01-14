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
    vec2 coords = tex_coords_frag;
        col = texture2D(tex, coords);
        float cornerh = min(1.0-coords.x,coords.x);
        float cornerl = min(coords.y,1.0-coords.y);
        float corner = min(cornerh, cornerl)*10.0;
        //corner += 0.1;
        col.a = corner;
        
	    color = vec4(vec3(0.0)*(1.0-dist)+vec3(0.95)*dist,col.a);
}
