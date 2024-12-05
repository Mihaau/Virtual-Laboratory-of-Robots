// src/sceneLoader.cpp
#include "sceneLoader.h"

SceneLoader::SceneLoader() {
    // Stwórz katalog scenes jeśli nie istnieje
    if (!fs::exists(scenesPath)) {
        fs::create_directories(scenesPath);
    }
    ScanDirectory();
}

void SceneLoader::ScanDirectory() {
    sceneFiles.clear();
    for (const auto& entry : fs::directory_iterator(scenesPath)) {
        if (entry.path().extension() == ".scn") {
            sceneFiles.push_back(entry.path().filename().string());
        }
    }
}

void SceneLoader::DrawImGuiControls() {
    if (ImGui::BeginTabItem("Sceny")) {
        if (ImGui::Button("Zapisz")) {
            //implementacja zapisu sceny do pliku
            ScanDirectory();
        }

        ImGui::Separator();

        for (const auto& file : sceneFiles) {
            ImGui::Text("%s", file.c_str());
            ImGui::SameLine();
            
            // Unikalny ID dla każdego przycisku
            std::string buttonId = "Załaduj##" + file;
            if (ImGui::Button(buttonId.c_str())) {
                // TODO: Implementacja ładowania sceny
            }
        }

        if (sceneFiles.empty()) {
            ImGui::TextWrapped("Brak zapisanych scen w %s", scenesPath.c_str());
        }

        ImGui::EndTabItem();
    }
}