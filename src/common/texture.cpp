#include "texture.h"
#include "stb_image.h"

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <GL/glew.h>

void load_image_hdr(texture *t, const char *filename)
{
    int image_width, image_height, image_components;
    float *image_data = stbi_loadf(filename, &image_width, &image_height, &image_components, 0);

    // create gl objects.
    GLenum internalFormat = GL_RGB32F;
    GLenum format = GL_RGB;
    GLenum type = GL_FLOAT;
    GLsizei width = image_width;
    GLsizei height = image_height;

    glCreateTextures(GL_TEXTURE_2D, 1, &t->id);
    glTextureStorage2D(t->id, 1, internalFormat, t->width, t->height);
    glTextureSubImage2D(t->id, 0, 0, 0, t->width, t->height, format, type, image_data);

    stbi_image_free(image_data);
}

