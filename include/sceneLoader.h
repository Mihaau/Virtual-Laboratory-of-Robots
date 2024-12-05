// include/sceneLoader.h
#ifndef SCENE_LOADER_H
#define SCENE_LOADER_H

#include <string>
#include <vector>
#include <filesystem>
#include "imgui.h"

namespace fs = std::filesystem;

class SceneLoader {
public:
    SceneLoader();
    void DrawImGuiControls();
    void ScanDirectory();

private:
    std::vector<std::string> sceneFiles;
    const std::string scenesPath = "assets/scenes";
};

#endif