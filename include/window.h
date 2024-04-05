#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

void createWindow(GLFWwindow **window, int width, int height, const char *title);
void framebuffer_size_callback(GLFWwindow *window, int width, int height);