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
            
            std::string loadButtonId = "Załaduj##" + file;
            if (ImGui::Button(loadButtonId.c_str())) {
                if (onLoadScene) {
                    std::string filename = file;
                    filename = filename.substr(0, filename.length() - 4);
                    onLoadScene(filename);
                }
            }
            
            ImGui::SameLine();
            std::string deleteButtonId = "Usuń##" + file;
            if (ImGui::Button(deleteButtonId.c_str())) {
                std::string filename = file;
                filename = filename.substr(0, filename.length() - 4);
                DeleteScene(filename);
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

    std::string filepath = (std::filesystem::path(scenesPath) / (filename + ".scn")).string();
    std::ofstream file(filepath);
    file << j.dump(4);
    
    ScanDirectory();
    return true;
}

bool SceneLoader::LoadScene(const std::string& filename, std::vector<Object3D*>& objects, Shader& shader) {
    if (!LoadSceneFile(filename)) {
        return false;
    }
    
    UpdateObjects(objects, shader);
    return true;
}

bool SceneLoader::LoadSceneFile(const std::string& filename) {
    std::string filepath = (std::filesystem::path(scenesPath) / (filename + ".scn")).string();
    std::ifstream file(filepath);
    
    if (!file.is_open()) {
        return false;
    }

    try {
        nlohmann::json j;
        file >> j;
        
        currentScene.objects.clear();
        
        for (const auto& objData : j["objects"]) {
            ObjectData obj;
            obj.modelPath = objData["modelPath"];
            
            obj.position.x = objData["position"][0];
            obj.position.y = objData["position"][1]; 
            obj.position.z = objData["position"][2];
            
            obj.rotation.x = objData["rotation"][0];
            obj.rotation.y = objData["rotation"][1];
            obj.rotation.z = objData["rotation"][2];
            
            obj.scale = objData["scale"];
            
            currentScene.objects.push_back(obj);
        }
        
        return true;
    }
    catch (nlohmann::json::exception& e) {
        return false;
    }
}

void SceneLoader::UpdateObjects(std::vector<Object3D*>& objects, Shader& shader) {
    // Usuń istniejące obiekty
    for (auto obj : objects) {
        obj->markedForDeletion = true;
    }
    objects.clear();
    
    // Dodaj nowe obiekty
    for (const auto& objData : currentScene.objects) {
        Object3D* obj = new Object3D(objData.modelPath.c_str(), shader);
        obj->SetPosition(objData.position);
        obj->SetRotation(objData.rotation); 
        obj->SetScale(objData.scale);
        objects.push_back(obj);
    }
}

void SceneLoader::DeleteScene(const std::string& filename) {
    std::string filepath = scenesPath + "/" + filename + ".scn";
    if (fs::exists(filepath)) {
        fs::remove(filepath);
        ScanDirectory(); // Odśwież listę scen
    }
}