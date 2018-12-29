#version 460 core

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inColor;
layout(location = 3) in vec2 inTexCoord;

out VS_OUT
{
    vec4 color;
    vec3 normal;
    vec2 tc;
} vs_out;

void main() 
{
    gl_Position = proj * view * model * vec4(inPosition.xyz,1);
    vs_out.color = vec4(inColor.rgb,1);
    mat4 modelViewInverseTranspose = transpose(inverse(view * model));
    vs_out.normal = (modelViewInverseTranspose * vec4(inNormal.xyz,0)).xyz;
    vs_out.tc = inTexCoord.xy;
}
