#pragma once

void createVertexShader(const char *vertexShaderSource, unsigned int *vertexShader);
void createFragmentShader(const char *fragmentShaderSource, unsigned int *fragmentShader);
void createShaderProgram(unsigned int vertexShader, unsigned int fragmentShader, unsigned int *shaderProgram);
class Shader
{
public:
    unsigned int ID;
    Shader(const char *vertexPath, const char *fragmentPath);
    void use();
    void deleteProgram();
    void setBool(const std::string &name, bool value) const;
    void setInt(const std::string &name, int value) const;
    void setFloat(const std::string &name, float value) const;

private:
    void checkCompileErrors(unsigned int shader, std::string type);
};