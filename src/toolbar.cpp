#include "toolbar.h"

ToolBar::ToolBar(float screenWidth) : screenWidth(screenWidth), buttonSize(32.0f) {
    buttonDim = ImVec2(120, buttonSize);
    
    buttons = {
        {ICON_FA_PLAY, "Start", "Uruchom program", [](){}},
        {ICON_FA_STOP, "Stop", "Zatrzymaj program", [](){}},
        {ICON_FA_ARROW_TREND_UP, "Debug", "Tryb debugowania", [](){}},
        {ICON_FA_FILE_CIRCLE_XMARK, "Reset", "Zresetuj program", [](){}},
        {ICON_FA_SPAGHETTI_MONSTER_FLYING, "Config", "Konfiguracja programu", [](){}}
    };
}

void ToolBar::Draw() {
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(screenWidth - 0.33f * screenWidth, 50));

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10, 10));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));

    ImGui::Begin("Toolbar", nullptr,
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoScrollbar);

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.3f, 0.3f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));

    for (size_t i = 0; i < buttons.size(); i++) {
        if (i > 0) ImGui::SameLine();
        DrawButton(buttons[i]);
    }

    ImGui::PopStyleColor(3);
    ImGui::End();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
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