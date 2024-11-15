#pragma once
#include "imgui.h"
#include "extras/IconsFontAwesome6.h"
#include <string>
#include <vector>
#include <functional>

class ToolBar
{
public:
    ToolBar(float screenWidth);
    ~ToolBar() = default;

    void Draw();
    void SetStartCallback(std::function<void()> callback);
    void SetStopCallback(std::function<void()> callback);
    void SetDebugCallback(std::function<void()> callback);
    void SetResetCallback(std::function<void()> callback);
    void SetConfigCallback(std::function<void()> callback);
    void UpdateScreenWidth(float newWidth)
    {
        screenWidth = newWidth;
        buttonDim = ImVec2(120, buttonSize); // Preserve button dimensions
    }

private:
    struct Button
    {
        std::string icon;
        std::string label;
        std::string tooltip;
        std::function<void()> callback;
    };

    float screenWidth;
    float buttonSize;
    ImVec2 buttonDim;
    std::vector<Button> buttons;

    void DrawButton(const Button &button);
};