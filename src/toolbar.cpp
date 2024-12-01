// toolbar.cpp
#include "toolbar.h"

ToolBar::ToolBar(float screenWidth) : screenWidth(screenWidth), buttonSize(32.0f) {
    buttonDim = ImVec2(120, buttonSize);
    
    buttons = {
        {ICON_FA_PLAY, " Start", "Uruchom program", [](){}},
        {ICON_FA_STOP, " Stop", "Zatrzymaj program", [](){}},
        {ICON_FA_BUG, " Debug", "Tryb debugowania", [](){}},
        {ICON_FA_ROTATE_RIGHT, " Reset", "Zresetuj program", [](){}},
        {ICON_FA_SPAGHETTI_MONSTER_FLYING, " Config", "Konfiguracja programu", [](){}}
    };
}

void ToolBar::Draw() {
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10, 10));
    
    // Używamy kolorów z motywu Catppuccin Mocha
    ImGui::Begin("Toolbar", nullptr,
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoScrollbar|
        ImGuiWindowFlags_NoScrollWithMouse);

    // Oblicz szerokość wszystkich przycisków razem
    float totalButtonWidth = buttons.size() * buttonDim.x + (buttons.size() - 1) * ImGui::GetStyle().ItemSpacing.x;
    float offsetX = (screenWidth - totalButtonWidth) * 0.5f;

    // Ustaw przesunięcie przed pierwszym przyciskiem
    ImGui::SetCursorPosX(offsetX);

    // Rysuj przyciski
    for (size_t i = 0; i < buttons.size(); i++) {
        if (i > 0) ImGui::SameLine();
        DrawButton(buttons[i]);
    }

    ImGui::End();
    ImGui::PopStyleVar();
}

void ToolBar::DrawButton(const Button& button) {
    if (ImGui::Button((button.icon + " " + button.label).c_str(), buttonDim)) {
        button.callback();
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("%s", button.tooltip.c_str());
    }
}

void ToolBar::SetStartCallback(std::function<void()> callback) {
    buttons[0].callback = callback;
}

void ToolBar::SetStopCallback(std::function<void()> callback) {
    buttons[1].callback = callback;
}

void ToolBar::SetDebugCallback(std::function<void()> callback) {
    buttons[2].callback = callback;
}

void ToolBar::SetResetCallback(std::function<void()> callback) {
    buttons[3].callback = callback;
}

void ToolBar::SetConfigCallback(std::function<void()> callback) {
    buttons[4].callback = callback;
}