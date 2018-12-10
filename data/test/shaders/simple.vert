#version 460 core

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

layout(location = 0) in vec3 inPosition;
//layout(location = 1) in vec3 inColor;
//layout(location = 2) in vec3 inNormal;
//layout(location = 3) in vec2 inTexCoord;
//
//layout(location = 0) out vec3 fragColor;
//layout(location = 1) out vec2 fragTexCoord;

//out gl_PerVertex 
//{
//    vec4 gl_Position;
//};

//layout(location = 7) in vec4 offset;

out VS_OUT
{
    vec4 color;
} vs_out;

void main() 
{
//    const vec4 vertices[3] = vec4[3](
//        vec4(0.25, -0.25, 0.5, 1.0),
//        vec4(-0.25, -0.25, 0.5, 1.0),
//        vec4(0.25, 0.25, 0.5, 1.0)
//    );
//
//    const vec4 colors[] = vec4[3](
//        vec4(1.0, 0.0, 0.0, 1.0),
//        vec4(0.0, 1.0, 0.0, 1.0),
//        vec4(0.0, 0.0, 1.0, 1.0));

//    gl_Position = vertices[gl_VertexID] + offset;
//    vs_out.color = colors[gl_VertexID];

    //gl_Position = proj * view * model * (vec4(inPosition,1) + offset);
    //vs_out.color = vec4(inColor,1);
    gl_Position = proj * view * model * vec4(inPosition,1);
    vs_out.color = vec4(1,0,1,1);

    //gl_Position = proj * view * model * vec4(inPosition, 1.0);
//    fragColor = inColor;
//    fragTexCoord = inTexCoord;
}
