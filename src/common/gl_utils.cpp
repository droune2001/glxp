#include "gl_utils.h"

#include "stb_image.h"

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

//
// TEXTURE
//

void load_image_hdr(GLuint *tex_id, const std::string &filename)
{
    stbi_set_flip_vertically_on_load(1);

    int image_width, image_height, image_components;
    float *image_data = stbi_loadf(filename.c_str(), &image_width, &image_height, &image_components, 0);

    GLenum internalFormat = GL_RGB32F;
    GLenum format = GL_RGB;
    GLenum type = GL_FLOAT;

    glCreateTextures(GL_TEXTURE_2D, 1, tex_id);
    glTextureStorage2D(*tex_id, 5, internalFormat, image_width, image_height); // 5 mip levels
    glTextureSubImage2D(*tex_id, 0, 0, 0, image_width, image_height, format, type, image_data); // upload first mip level
    glGenerateTextureMipmap(*tex_id);

    // constrain sampler (or texture sampler?)
    glTextureParameteri(*tex_id, GL_TEXTURE_BASE_LEVEL, 0);
    glTextureParameteri(*tex_id, GL_TEXTURE_MAX_LEVEL, 4);

    stbi_image_free(image_data);
}

} // namespace glutils
