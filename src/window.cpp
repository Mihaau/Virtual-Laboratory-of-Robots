#include "window.h"
#include "application.h"

void createWindow(GLFWwindow **window, int width, int height, const char *title)
{

    // Initialize GLFW
    if (!glfwInit())
    {
        std::cout << "Failed to initialize GLFW \n";
        exit(EXIT_FAILURE);
    }

    // Set the OpenGL version to 4.6
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    

    // For MacOS
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // Create a window
    *window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    if (*window == nullptr)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    glfwMakeContextCurrent(*window);
    glfwSwapInterval(1); // Enable vsync

}

// Callback function for resizing the window
void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    // Set the viewport size
    glViewport(0, 0, width, height);
}