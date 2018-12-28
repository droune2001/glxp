#version 460 core

layout(binding = 0) uniform sampler2D s;

in VS_OUT
{
    vec4 color;
    vec3 normal;
    vec2 tc;
} fs_in;

layout(location = 0) out vec4 outColor;

void main()
{
    vec4 tex_color = texture(s, fs_in.tc);
    //outColor = fs_in.color;
    vec4 normal_color = vec4(vec3(0.5) * (fs_in.normal + vec3(1)),1);
    vec4 texcoords_color = vec4(fs_in.tc, 0, 1);

    //outColor = mix(normal_color, texcoords_color, 0.5);
    //outColor = mix(normal_color, tex_color, 0.5);
    //outColor = vec4(tex_color.rgb, 1.0);
    outColor = normal_color;
}
