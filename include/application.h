#pragma once

#include <GLFW/glfw3.h>

class UseImGui {
    public:
        void Init(GLFWwindow** window);
        void NewFrame();
        virtual void Update();
        void Render();
        void Shutdown();
        void Win1();
        void Win2();
};
