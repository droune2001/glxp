#include "gl_utils.h"

#include <GL/glew.h>
#include <string>
#include <vector>
#include <iostream>

namespace glutils
{

void check_error()
{
    int error = glGetError();
    if (error == GL_NO_ERROR)
    {
        return;
    }

    std::string errorStr;
    switch (error)
    {
        case GL_INVALID_ENUM:
            errorStr = "GL_INVALID_ENUM";
            break;
        case GL_INVALID_VALUE:
            errorStr = "GL_INVALID_VALUE";
            break;
        case GL_INVALID_OPERATION:
            errorStr = "GL_INVALID_OPERATION";
            break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            errorStr = "GL_INVALID_FRAMEBUFFER_OPERATION";
            break;
        case GL_OUT_OF_MEMORY:
            errorStr = "GL_OUT_OF_MEMORY";
            break;
        default:
            errorStr = "UNKNOWN";
            break;
    }

    printf("gl error: %s\n", errorStr.c_str());
}

bool compile_shader(GLuint shader, const char* buffer, size_t bufferSize)
{
    GLint length = (GLint)bufferSize;
    glShaderSource(shader, 1, &buffer, &length);
    glCompileShader(shader);

    int logSize;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logSize);

    if (logSize > 1)
    {
        std::vector<char> log(logSize);
        glGetShaderInfoLog(shader, logSize, &logSize, log.data());
        std::cout << buffer << std::endl;
        std::cout << "Compile: " << log.data() << std::endl;
    }

    int status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    return (status != GL_FALSE);
}

bool link_program(GLuint program, GLuint vertexShader, GLuint fragmentShader)
{
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    int logSize;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logSize);

    if (logSize > 1)
    {
        std::vector<char> log(logSize);
        glGetProgramInfoLog(program, logSize, &logSize, log.data());
        std::cout << "Link: " << log.data() << std::endl;
    }

    int status;
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    return (status != GL_FALSE);
}

} // namespace glutils
