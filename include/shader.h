#pragma once

#include <glad/glad.h>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>

class Shader
{
public:
    GLuint ID;
    Shader(const char *vertexPath, const char *fragmentPath);
    void use();
    void deleteProgram();
    void setBool(const std::string &name, bool value) const;
    void setInt(const std::string &name, int value) const;
    void setFloat(const std::string &name, float value) const;
private:
    void checkCompileErrors(GLuint shader, std::string type);
};