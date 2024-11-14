#pragma once
#include <string>
#include "imgui.h"
#include "raylib.h"

class CodeEditor {
private:
    std::string content;
    char* buffer;
    size_t bufferSize;
    bool showLineNumbers;
    std::string filename;
    ImGuiInputTextFlags flags;
    
public:
    CodeEditor(size_t initialBufferSize = 8192);
    ~CodeEditor();
    
    void Draw(const char* label);
    void LoadFromFile(const std::string& path);
    void SaveToFile(const std::string& path);
    const std::string& GetContent() const;
    void SetContent(const std::string& text);
};