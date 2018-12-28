#ifndef _GL_UTILS_2018_12_04_H_
#define _GL_UTILS_2018_12_04_H_

#include <GL/glew.h>
#include <string>

namespace glutils
{
    void check_error();

    bool compile_shader(GLuint shader, const char* buffer, size_t bufferSize);
    bool link_program(GLuint program, GLuint vertexShader, GLuint fragmentShader);

    void load_image_hdr(GLuint *tex_id, const std::string &filename);
}

#endif // _GL_UTILS_2018_12_04_H_
