#pragma once
#include <vector>
#include <string>
#include "imgui.h"

enum class LogLevel {
    Info,
    Warning,
    Error
};

struct LogMessage {
    std::string message;
    LogLevel level;
    float time;
};

class LogWindow {
public:
    LogWindow();
    void Draw(const char* title, const ImVec2& position, const ImVec2& size);
    void AddLog(const char* message, LogLevel level = LogLevel::Info);
    void Clear();

private:
    std::vector<LogMessage> logs;
    bool autoScroll;
    ImGuiTextFilter filter;
    
    ImVec4 GetColorForLevel(LogLevel level) const;
    const char* GetPrefixForLevel(LogLevel level) const;
};