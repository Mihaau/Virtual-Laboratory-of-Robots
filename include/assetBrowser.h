#pragma once
#include "raylib.h"
#include <string>
#include <vector>
#include <filesystem>
#include "lightController.h"
#include "raymath.h"
#include "rlImGui.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <iomanip>
#include "modelConfig.h"

using json = nlohmann::json;
namespace fs = std::filesystem;

struct AssetItem {
    std::string name;
    std::string path;
    Model model;
    RenderTexture2D thumbnail;
    ModelConfig config;
};

class AssetBrowser {
public:
    AssetBrowser();
    ~AssetBrowser();
    void DrawImGuiControls();

private:
    std::vector<AssetItem> assets;
    Camera previewCamera;
    Shader shader;
    LightController* lightController;

    bool showConfigEditor = false;
    AssetItem* selectedItem = nullptr;
    ModelConfig editingConfig;

    void ScanDirectory(const std::string& path);
    void GenerateThumbnail(AssetItem& item);
    void LoadDescription(AssetItem& item);
};