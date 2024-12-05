#pragma once
#include "imgui.h"
#include "extras/IconsFontAwesome6.h"
#include <string>
#include <vector>
#include <functional>
#include "logWindow.h"

class ToolBar
{
public:
    ToolBar(float screenWidth);
    ~ToolBar() = default;

    void Draw();
    void SetStartCallback(std::function<void()> callback);
    void SetPauseCallback(std::function<void()> callback);
    void SetStepCallback(std::function<void()> callback);
    void SetResetCallback(std::function<void()> callback);
    void UpdateScreenWidth(float newWidth)
    {
        screenWidth = newWidth;
        buttonDim = ImVec2(120, buttonSize);
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
    LogWindow logWindow;

    void DrawButton(const Button &button);
};