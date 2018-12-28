#version 460 core

layout(binding = 0) uniform sampler2D s;

in VS_OUT
{
    vec2 tc;
} fs_in;

layout(location = 0) out vec4 outColor;

vec3 tonemap(vec3 hdr)
{
    return clamp(hdr, vec3(0), vec3(1));
}

void main()
{
    vec3 hdr = texture(s, fs_in.tc).rgb;
    vec3 ldr = tonemap(hdr);
    outColor = vec4(ldr,1);
    //outColor = vec4(1,0,0,1);
}
