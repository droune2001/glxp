#version 460 core

layout(binding = 1) uniform sampler3D lut_sampler;

in VS_OUT
{
    vec3 tc;
} fs_in;

layout(location = 0) out vec4 outColor;

void main()
{
    //int x = mod(int(fs_in.tc.z * 32.0), 32);
    int scaled_z = int(fs_in.tc.z * 32.0);
    int x = scaled_z - 32 * (scaled_z/32);
    int y = int(fs_in.tc.y);
    int z = int(fs_in.tc.z);
    ivec3 base_tc = ivec3(x,y,z);
    outColor = texelFetch(lut_sampler, base_tc, 0);
}
