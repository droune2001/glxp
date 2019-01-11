#version 460 core

uniform int width;
uniform int height;

out VS_OUT
{
    vec3 tc;
} vs_out;

const vec2 pos_data[6] = vec2[] 
(
    vec2( 0.0,  0.0),
    vec2( 1.0,  0.0),
    vec2( 0.0,  1.0),
    vec2( 0.0,  1.0),
    vec2( 1.0,  0.0),
    vec2( 1.0,  1.0)
);

const vec3 tex_data[6] = vec3[] 
(
    vec3(0.0, 32.0,  0.0),
    vec3(0.0, 32.0, 32.0),
    vec3(0.0,  0.0,  0.0),
    vec3(0.0,  0.0,  0.0),
    vec3(0.0, 32.0, 32.0),
    vec3(0.0,  0.0, 32.0)
);

void main() 
{
    vec2 pixel_size = vec2(2.0/float(width), 2.0/(float(height)));
    vec2 scale = vec2(1024.0*pixel_size.x, -32.0*pixel_size.y);
    vec2 offset = vec2(-1.0, -1.0+32.0*pixel_size.y);
    gl_Position = vec4( offset + scale * pos_data[ gl_VertexID ], 0.0, 1.0 );
    vs_out.tc = tex_data[ gl_VertexID ];
}
