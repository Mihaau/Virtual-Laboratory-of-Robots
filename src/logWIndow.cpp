#include "logWindow.h"
#include "raylib.h"

LogWindow::LogWindow() : autoScroll(true) {}

void LogWindow::Draw(const char* title, const ImVec2& position, const ImVec2& size) {
    ImGui::SetNextWindowPos(position);
    ImGui::SetNextWindowSize(size);
    
    if (ImGui::Begin(title, nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse)) {
        // Toolbar
        if (ImGui::Button("Clear")) Clear();
        ImGui::SameLine();
        ImGui::Checkbox("Auto-scroll", &autoScroll);
        ImGui::SameLine();
        filter.Draw("Filter", -100.0f);
        
        ImGui::Separator();
        
        // Log content
        ImGui::BeginChild("ScrollingRegion", ImVec2(0, 0), false, 
            ImGuiWindowFlags_HorizontalScrollbar);
        
        for (const auto& log : logs) {
            if (!filter.PassFilter(log.message.c_str())) 
                continue;
                
            ImGui::PushStyleColor(ImGuiCol_Text, GetColorForLevel(log.level));
            ImGui::TextUnformatted(GetPrefixForLevel(log.level));
            ImGui::SameLine();
            ImGui::TextUnformatted(log.message.c_str());
            ImGui::PopStyleColor();
        }
        
        if (autoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
            ImGui::SetScrollHereY(1.0f);
        }
        
        ImGui::EndChild();
    }
    ImGui::End();
}

void LogWindow::AddLog(const char* message, LogLevel level) {
    logs.push_back({message, level, static_cast<float>(GetTime())});
}

void LogWindow::Clear() {
    logs.clear();
}

ImVec4 LogWindow::GetColorForLevel(LogLevel level) const {
    switch (level) {
        case LogLevel::Info:    return ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
        case LogLevel::Warning: return ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
        case LogLevel::Error:   return ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
        default:                return ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    }
}

const char* LogWindow::GetPrefixForLevel(LogLevel level) const {
    switch (level) {
        case LogLevel::Info:    return "[INFO]";
        case LogLevel::Warning: return "[WARN]";
        case LogLevel::Error:   return "[ERROR]";
        default:                return "[INFO]";
    }
}