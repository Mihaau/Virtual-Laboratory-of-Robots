#include "toolbar.h"

ToolBar::ToolBar(float screenWidth)
    : screenWidth(screenWidth), buttonSize(32.0f), logWindow(LogWindow::GetInstance()) // Inicjalizacja referencji do singletona
{
    buttonDim = ImVec2(120, buttonSize);

    buttons = {
        {ICON_FA_PLAY, " Start", "Uruchom program", []() {}},
        {ICON_FA_PAUSE, " Pauza", "Wstrzymaj wykonywanie", []() {}},
        {ICON_FA_FORWARD_STEP, " Krok", "Wykonaj jeden krok", []() {}}
        // {ICON_FA_ROTATE_RIGHT, " Reset", "Zresetuj program", []() {}}
        };
}

void ToolBar::Draw()
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10, 10));

    ImGui::Begin("Toolbar", nullptr,
                 ImGuiWindowFlags_NoMove |
                     ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoTitleBar |
                     ImGuiWindowFlags_NoScrollbar |
                     ImGuiWindowFlags_NoScrollWithMouse);

    float totalButtonWidth = buttons.size() * buttonDim.x + (buttons.size() - 1) * ImGui::GetStyle().ItemSpacing.x;
    float offsetX = (screenWidth - totalButtonWidth) * 0.5f;

    ImGui::SetCursorPosX(offsetX);

    for (size_t i = 0; i < buttons.size(); i++)
    {
        if (i > 0)
            ImGui::SameLine();
        DrawButton(buttons[i]);
    }

    ImGui::End();
    ImGui::PopStyleVar();
}

void ToolBar::DrawButton(const Button &button)
{
    if (ImGui::Button((button.icon + " " + button.label).c_str(), buttonDim))
    {
        button.callback();
    }
    if (ImGui::IsItemHovered())
    {
        ImGui::SetTooltip("%s", button.tooltip.c_str());
    }
}

void ToolBar::SetStartCallback(std::function<void()> callback)
{
    buttons[0].callback = [this, callback]()
    {
        logWindow.AddLog("Uruchamianie programu", LogLevel::Info);
        callback();
    };
}

void ToolBar::SetPauseCallback(std::function<void()> callback)
{
    buttons[1].callback = callback;
}

void ToolBar::SetStepCallback(std::function<void()> callback)
{
    buttons[2].callback = [this, callback]()
    {
        logWindow.AddLog("Wykonywanie kroku", LogLevel::Info);
        callback();
    };
}

void ToolBar::SetResetCallback(std::function<void()> callback)
{
    buttons[3].callback = callback;
}