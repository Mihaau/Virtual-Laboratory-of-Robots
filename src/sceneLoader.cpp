// src/sceneLoader.cpp
#include "sceneLoader.h"

using json = nlohmann::json;

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
        static char sceneName[128] = "";
        
        // Pole tekstowe i przycisk zapisu
        ImGui::SetNextItemWidth(200);
        ImGui::InputText("##NazwaSceny", sceneName, IM_ARRAYSIZE(sceneName));
        ImGui::SameLine();
        
        if (ImGui::Button("Zapisz")) {
            if (strlen(sceneName) > 0) {
                if (onSaveScene) {
                    onSaveScene(sceneName);
                }
                memset(sceneName, 0, sizeof(sceneName));
                ScanDirectory();
            }
        }

        ImGui::Separator();

        // Lista zapisanych scen
        for (const auto& file : sceneFiles) {
            ImGui::Text("%s", file.c_str());
            ImGui::SameLine();
            
            std::string buttonId = "Załaduj##" + file;
            if (ImGui::Button(buttonId.c_str())) {
                if (onLoadScene) {
                    // Usuwamy rozszerzenie .scn z nazwy pliku
                    std::string filename = file;
                    filename = filename.substr(0, filename.length() - 4);
                    onLoadScene(filename);
                }
            }
        }

        if (sceneFiles.empty()) {
            ImGui::TextWrapped("Brak zapisanych scen w %s", scenesPath.c_str());
        }

        ImGui::EndTabItem();
    }
}

bool SceneLoader::SaveScene(const std::string& filename, const std::vector<Object3D*>& objects) {
    SceneData sceneData;
    
    for (const auto& obj : objects) {
        ObjectData objData;
        objData.modelPath = obj->GetModelPath();
        objData.position = obj->GetPosition();
        objData.rotation = obj->GetRotation();
        objData.scale = obj->GetScale();
        sceneData.objects.push_back(objData);
    }

    nlohmann::json j;
    for (const auto& obj : sceneData.objects) {
        nlohmann::json objJson;
        objJson["modelPath"] = obj.modelPath;
        objJson["position"] = {obj.position.x, obj.position.y, obj.position.z};
        objJson["rotation"] = {obj.rotation.x, obj.rotation.y, obj.rotation.z};
        objJson["scale"] = obj.scale;
        j["objects"].push_back(objJson);
    }

    std::string filepath = scenesPath + "/" + filename + ".scn";
    std::ofstream file(filepath);
    file << j.dump(4);
    
    ScanDirectory();
    return true;
}

bool SceneLoader::LoadScene(const std::string& filename, std::vector<Object3D*>& objects, Shader& shader) {
    std::string filepath = scenesPath + "/" + filename;
    std::ifstream file(filepath);
    
    if (!file.is_open()) return false;

    json j;
    file >> j;

    // Usuń istniejące obiekty
    for (auto* obj : objects) {
        delete obj;
    }
    objects.clear();

    // Wczytaj nowe obiekty
    for (const auto& objJson : j["objects"]) {
        std::string modelPath = objJson["modelPath"];
        Vector3 position = {
            objJson["position"][0],
            objJson["position"][1],
            objJson["position"][2]
        };
        Vector3 rotation = {
            objJson["rotation"][0],
            objJson["rotation"][1],
            objJson["rotation"][2]
        };
        float scale = objJson["scale"];

        Object3D* obj = Object3D::Create(modelPath.c_str(), shader);
        obj->SetPosition(position);
        obj->SetRotation(rotation);
        obj->SetScale(scale);
        objects.push_back(obj);
    }

    return true;
}