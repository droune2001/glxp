#ifndef _GL_UTILS_2018_12_04_H_
#define _GL_UTILS_2018_12_04_H_

#include <GL/glew.h>

namespace glutils
{
    void check_error();

    void compile_shader(GLuint shader, const char* buffer, size_t bufferSize);
    void link_program(GLuint program, GLuint vertexShader, GLuint fragmentShader);
}

#endif // _GL_UTILS_2018_12_04_H_
