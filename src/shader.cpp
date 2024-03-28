#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

void createVertexShader(const char *vertexShaderSource, unsigned int *vertexShader)
{
    // Create a vertex shader
    *vertexShader = glCreateShader(GL_VERTEX_SHADER);
    // Attach the shader source code to the shader object
    glShaderSource(*vertexShader, 1, &vertexShaderSource, NULL);
    // Compile the shader
    glCompileShader(*vertexShader);
    // Check for compilation errors
    int success;
    char infoLog[512];
    glGetShaderiv(*vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(*vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n"
                  << infoLog << "\n";
    }
}

void createFragmentShader(const char *fragmentShaderSource, unsigned int *fragmentShader)
{
    // Create a fragment shader
    *fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    // Attach the shader source code to the shader object
    glShaderSource(*fragmentShader, 1, &fragmentShaderSource, NULL);
    // Compile the shader
    glCompileShader(*fragmentShader);
    // Check for compilation errors
    int success;
    char infoLog[512];
    glGetShaderiv(*fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(*fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n"
                  << infoLog << "\n";
    }
}

void createShaderProgram(unsigned int vertexShader, unsigned int fragmentShader, unsigned int *shaderProgram)
{
    // Create a shader program
    *shaderProgram = glCreateProgram();
    // Attach the vertex shader to the shader program
    glAttachShader(*shaderProgram, vertexShader);
    // Attach the fragment shader to the shader program
    glAttachShader(*shaderProgram, fragmentShader);
    // Link the shader program
    glLinkProgram(*shaderProgram);
    // Check for linking errors
    int success;
    char infoLog[512];
    glGetProgramiv(*shaderProgram, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(*shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n"
                  << infoLog << "\n";
    }

    // Delete the shaders
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}