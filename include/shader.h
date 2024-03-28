#pragma once

void createVertexShader(const char *vertexShaderSource, unsigned int *vertexShader);
void createFragmentShader(const char *fragmentShaderSource, unsigned int *fragmentShader);
void createShaderProgram(unsigned int vertexShader, unsigned int fragmentShader, unsigned int *shaderProgram);