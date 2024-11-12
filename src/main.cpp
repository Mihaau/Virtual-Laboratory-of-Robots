#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define STB_IMAGE_IMPLEMENTATION
#include <iostream>
#include "shader.h"
#include "window.h"
#include "texture.h"
#include "VAO.h"
#include "VBO.h"
#include "EBO.h"

int main()
{
    GLFWwindow *window = nullptr;
    createWindow(&window, 1280 , 720, "Virtual Laboratory of Robots");
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Load GLAD
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    Shader shader("assets/shaders/vertex_shader.vert", "assets/shaders/fragment_shader.frag");
    Texture texture("assets/images/image1.jpg");
    Texture texture2("assets/images/image2.jpg");

    // Create a vertex array object (x, y, z)
    GLfloat vertices[] = {
        // positions          // colors           // texture coords
        0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,   // top right
        0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,  // bottom right
        -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom left
        -0.5f, 0.5f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f   // top left
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
    vao.LinkAttrib(vbo, 0, 3, GL_FLOAT, 8 * sizeof(float), (void *)nullptr);
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

        // Create transformations
        auto trans = glm::mat4(1.0f);
        trans = glm::translate(trans, glm::vec3(0.0f, 0.0f, 0.0f));
        trans = glm::rotate(trans, (float)glfwGetTime(), glm::vec3(0.0f, 0.0f, 1.0f));

        unsigned int transformLoc = glGetUniformLocation(shader.ID, "transform");
        glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(trans));

        vao.Bind();
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

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