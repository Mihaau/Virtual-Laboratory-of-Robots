#include "codeEditor.h"
#include <fstream>
#include <sstream>

CodeEditor::CodeEditor(size_t initialBufferSize) : bufferSize(initialBufferSize), showLineNumbers(true) {
    buffer = new char[bufferSize];
    memset(buffer, 0, bufferSize);
    flags = ImGuiInputTextFlags_AllowTabInput | 
            ImGuiInputTextFlags_None;
}

CodeEditor::~CodeEditor() {
    delete[] buffer;
}

void CodeEditor::Draw(const char* label) {
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 8));
    
    // Editor settings
    if (ImGui::CollapsingHeader("Ustawienia edytora")) {
        ImGui::Checkbox("Numery linii", &showLineNumbers);
        
        if (ImGui::Button("Zapisz")) {
            if (!filename.empty()) {
                SaveToFile(filename);
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Wczytaj")) {
            if (!filename.empty()) {
                LoadFromFile(filename);
            }
        }
    }

    // Main editor window
    float reserveHeight = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
    ImVec2 size = ImGui::GetContentRegionAvail();
    size.y -= reserveHeight;

    // Line numbers
    if (showLineNumbers) {
        ImGui::BeginChild("##line_numbers", ImVec2(30, size.y), true);
        int lineCount = 1;
        for (const char* c = buffer; *c != '\0'; c++) {
            if (*c == '\n') {
                ImGui::Text("%d", lineCount++);
            }
        }
        ImGui::EndChild();
        ImGui::SameLine();
        size.x -= 30;
    }

    // Code editor
    ImGui::InputTextMultiline(label, 
        buffer, 
        bufferSize, 
        size,
        flags);

    ImGui::PopStyleVar();
}

void CodeEditor::LoadFromFile(const std::string& path) {
    filename = path;
    std::ifstream file(path);
    if (file.is_open()) {
        std::stringstream buffer;
        buffer << file.rdbuf();
        SetContent(buffer.str());
        file.close();
    }
}

void CodeEditor::SaveToFile(const std::string& path) {
    std::ofstream file(path);
    if (file.is_open()) {
        file << buffer;
        file.close();
    }
}

const std::string& CodeEditor::GetContent() const {
    return content;
}

void CodeEditor::SetContent(const std::string& text) {
    strncpy(buffer, text.c_str(), bufferSize - 1);
    buffer[bufferSize - 1] = '\0';
    content = text;
}