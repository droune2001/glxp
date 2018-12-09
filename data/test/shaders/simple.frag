#version 460 core

//uniform sampler2D tex;

//layout(location = 0) in vec3 fragColor;
//layout(location = 1) in vec2 fragTexCoord;

in VS_OUT
{
    vec4 color;
} fs_in;

layout(location = 0) out vec4 outColor;

void main()
{
    //outColor = texture(tex, fragTexCoord);
    //outColor = vec4(fragColor,1);
    outColor = fs_in.color; 
}
