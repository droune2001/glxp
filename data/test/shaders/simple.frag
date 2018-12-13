#version 460 core

//uniform sampler2D tex;

in VS_OUT
{
    vec4 color;
    vec3 normal;
    vec2 tc;
} fs_in;

layout(location = 0) out vec4 outColor;

void main()
{
    //outColor = texture(tex, fragTexCoord);
    //outColor = fs_in.color;
    outColor = vec4(vec3(0.5) * (fs_in.normal + vec3(1)),1);
    //outColor = vec4(fs_in.tc, 0, 0);
}
