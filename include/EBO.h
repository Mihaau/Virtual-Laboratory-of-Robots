#pragma once

#include <glad/glad.h>

class EBO
{
public:
    // Reference ID of the Element Buffer Object
    GLuint ID;
    // Constructor that generates a Element Buffer Object and links it to indices
    EBO(GLuint *indices, GLsizeiptr size);
    // Binds the EBO
    void Bind();
    // Unbinds the EBO
    void Unbind();
    // Deletes the EBO
    void Delete();
};