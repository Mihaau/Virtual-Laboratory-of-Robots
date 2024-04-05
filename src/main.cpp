#include <glad/glad.h>
#include <GLFW/glfw3.h>
#define STB_IMAGE_IMPLEMENTATION
#include <iostream>
#include "shader.h"
#include "window.h"
#include "texture.h"
#include "EBO.h"
#include "VAO.h"
#include "VBO.h"

int main()
{
    GLFWwindow *window = nullptr;
    createWindow(&window, 800, 600, "Virtual Laboratory of Robots");
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Load GLAD
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    Shader shader("assets/shaders/vertex_shader.vert", "assets/shaders/fragment_shader.frag");
    Texture texture("assets/images/image1.jpg");
    Texture texture2("assets/images/image2.jpg");

    // Create a vertex array object (x, y, z)
    GLfloat vertices[] = {
        // positions          // colors           // texture coords
        1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,   // top right
        1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,  // bottom right
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom left
        -1.0f, 1.f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f    // top left
    };
    GLuint indices[] = {
        0, 1, 3, // first triangle
        1, 2, 3  // second triangle
    };

    // Create a vertex array object
    VAO vao;
    vao.Bind();

    // Create a vertex buffer object and element buffer object
    VBO vbo(vertices, sizeof(vertices));
    EBO ebo(indices, sizeof(indices));

    // Link VBO and EBO to VAO
    vao.LinkAttrib(vbo, 0, 3, GL_FLOAT, 8 * sizeof(float), (void *)0);
    vao.LinkAttrib(vbo, 1, 3, GL_FLOAT, 8 * sizeof(float), (void *)(3 * sizeof(float)));
    vao.LinkAttrib(vbo, 2, 2, GL_FLOAT, 8 * sizeof(float), (void *)(6 * sizeof(float)));

    // Unbind the VAO, VBO and EBO
    vao.Unbind();
    vbo.Unbind();
    ebo.Unbind();

    // wireframe mode
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    while (!glfwWindowShouldClose(window))
    {
        glClearColor(0.2f, 0.8f, 0.8f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        texture.bind();
        texture2.bind(1);
        shader.use();
        shader.setInt("texture", 0);
        shader.setInt("texture2", 1);

        vao.Bind();
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Delete the VAO, VBO, EBO and the shader program
    vao.Delete();
    vbo.Delete();
    ebo.Delete();

    shader.deleteProgram();

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}