#pragma once

class Texture
{
public:
    unsigned int ID;
    Texture(const char *path);
    ~Texture();
    void bind(int slot = 0);
    void unbind();
};
