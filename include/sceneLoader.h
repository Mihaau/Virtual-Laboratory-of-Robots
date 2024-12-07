// include/sceneLoader.h
#ifndef SCENE_LOADER_H
#define SCENE_LOADER_H

#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include <functional>
#include "raylib.h"  // dla Vector3
#include "imgui.h"
#include "object3D.h" // dla Object3D
#include <nlohmann/json.hpp>

namespace fs = std::filesystem;

struct ObjectData {
    std::string modelPath;
    Vector3 position;
    Vector3 rotation;
    float scale;
};

struct SceneData {
    std::vector<ObjectData> objects;
};

class SceneLoader {
public:
    SceneLoader();
    void DrawImGuiControls();
    void ScanDirectory();
    bool SaveScene(const std::string& filename, const std::vector<Object3D*>& objects);
    bool LoadScene(const std::string& filename, std::vector<Object3D*>& objects, Shader& shader);

    std::function<void(const std::string&)> onSaveScene;
    std::function<void(const std::string&)> onLoadScene;

private:
    std::vector<std::string> sceneFiles;
    const std::string scenesPath = "assets/scenes";
    SceneData currentScene;
};

#endif // SCENE_LOADER_H