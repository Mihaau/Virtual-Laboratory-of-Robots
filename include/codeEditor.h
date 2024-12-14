#pragma once
#include <string>
#include "imgui.h"
#include "raylib.h"
#include <filesystem>
#include <vector>

class CodeEditor {
private:
    std::string content;
    char* buffer;
    size_t bufferSize;
    bool showLineNumbers;
    std::string filename;
    ImGuiInputTextFlags flags;
    int currentLine = 0;
        const std::string scriptsPath = "assets/scripts";
    std::vector<std::string> scriptFiles;
    void ScanScriptsDirectory();
    
public:
    CodeEditor(size_t initialBufferSize = 8192);
    ~CodeEditor();
    
    void Draw(const char* label);
    void LoadFromFile(const std::string& path);
    void SaveToFile(const std::string& path);
    const std::string& GetContent() const;
    void SetContent(const std::string& text);

        void SetCurrentLine(int line);
    int GetCurrentLine() const;
    const char* GetText() const { return buffer; }
        void SaveScript(const std::string& filename);
    void LoadScript(const std::string& filename);
    void DeleteScript(const std::string& filename);
};