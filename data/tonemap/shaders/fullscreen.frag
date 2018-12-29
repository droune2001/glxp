#version 460 core

layout(binding = 0) uniform sampler2D s;

in VS_OUT
{
    vec2 tc;
} fs_in;

layout(location = 0) out vec4 outColor;

void main()
{
    outColor = texture(s, fs_in.tc);
}
