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
    
    if (ImGui::CollapsingHeader("Ustawienia edytora")) {
        ImGui::Checkbox("Numery linii", &showLineNumbers);
        
        if (ImGui::Button("Zapisz") && !filename.empty()) {
            SaveToFile(filename);
        }
        ImGui::SameLine();
        if (ImGui::Button("Wczytaj") && !filename.empty()) {
            LoadFromFile(filename);
        }
    }

    float reserveHeight = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
    ImVec2 size = ImGui::GetContentRegionAvail();
    size.y -= reserveHeight;

    if (showLineNumbers) {
        ImGui::BeginChild("##line_numbers", ImVec2(30, size.y), true);
        
        // Oblicz całkowitą liczbę linii
        int totalLines = 1;
        for (const char* c = buffer; *c != '\0'; c++) {
            if (*c == '\n') totalLines++;
        }

        // Pobierz style i metryki
        float lineHeight = ImGui::GetTextLineHeight();
        float padding = ImGui::GetStyle().FramePadding.y;
        
        // Ustaw początkową pozycję Y uwzględniającą padding
        float currentY = padding;
        
        // Renderuj numery linii
        for (int i = 1; i <= totalLines; i++) {
            ImGui::SetCursorPosY(currentY);
            ImGui::Text("%d", i);
            currentY += lineHeight;
        }
        
        ImGui::EndChild();
        ImGui::SameLine();
        size.x -= 30;
    }

    ImGui::InputTextMultiline(label, buffer, bufferSize, size, flags);
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