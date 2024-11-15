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
using json = nlohmann::json;

struct ModelConfig {
    struct {
        float scale;
        struct { float x, y, z; } rotation;
        struct { float x, y, z; } position;
    } model;
    
    struct {
        struct { int r, g, b, a; } background;
        struct { int width, height; } size;
        struct {
            struct { float x, y, z; } position;
            struct { float x, y, z; } target;
            float fov;
        } camera;
    } thumbnail;
    
    struct {
        struct { float r, g, b, a; } ambient;
    } materials;
};

struct AssetItem {
    std::string name;
    std::string path;
    Model model;
    RenderTexture2D thumbnail;
    std::string cachePath;
    ModelConfig config;
};

class AssetBrowser {
private:
    std::vector<AssetItem> assets;
    Camera previewCamera;
    Shader shader;
    LightController* lightController;
    
    void GenerateThumbnail(AssetItem& item);
    bool LoadCachedThumbnail(AssetItem& item);
    void SaveThumbnail(const AssetItem& item);
    std::string GetCachePath(const std::string& modelName);

public:
    AssetBrowser();
    ~AssetBrowser();
    void ScanDirectory(const std::string& path);
    void DrawImGuiControls();
    void LoadConfig(AssetItem& item);
};