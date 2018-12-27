#ifndef _TEXTURE_2017_12_27_H_
#define _TEXTURE_2017_12_27_H_

// TODO: move to glutils

struct texture
{
    unsigned int id;
    int width, height;
};

void load_image_hdr(texture *t, const char *filename);

#endif // _TEXTURE_2017_12_27_H_
