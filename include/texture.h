#pragma once

#include <glad/glad.h>
#include <iostream>
#include <stb_image.h>

class Texture
{
public:
    GLuint ID;
    Texture(const char *path);
    ~Texture();
    void bind(int slot = 0);
    void unbind();
};
