// assetBrowser.h
#pragma once
#include "raylib.h"
#include <string>
#include <vector>
#include <filesystem>

struct AssetItem {
    std::string name;
    std::string path;
    Model model;
    RenderTexture2D thumbnail;
};

class AssetBrowser {
private:
    std::vector<AssetItem> assets;
    Camera previewCamera;
    void GenerateThumbnail(AssetItem& item);

public:
    AssetBrowser();
    ~AssetBrowser();
    void ScanDirectory(const std::string& path);
    void DrawImGuiControls();
};

// assetBrowser.cpp
#include "assetBrowser.h"

AssetBrowser::AssetBrowser() {
    previewCamera = { 0 };
    previewCamera.position = (Vector3){ 10.0f, 10.0f, 10.0f };
    previewCamera.target = (Vector3){ 0.0f, 5.0f, 0.0f };
    previewCamera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    previewCamera.fovy = 45.0f;
    previewCamera.projection = CAMERA_PERSPECTIVE;
    
    ScanDirectory("assets/models");
}

void AssetBrowser::ScanDirectory(const std::string& path) {
    for (const auto& entry : std::filesystem::directory_iterator(path)) {
        if (entry.path().extension() == ".glb" || entry.path().extension() == ".obj") {
            AssetItem item;
            item.name = entry.path().filename().string();
            item.path = entry.path().string();
            item.model = LoadModel(item.path.c_str());
            item.thumbnail = LoadRenderTexture(128, 128);
            GenerateThumbnail(item);
            assets.push_back(item);
        }
    }
}

void AssetBrowser::GenerateThumbnail(AssetItem& item) {
    BeginTextureMode(item.thumbnail);
    ClearBackground(RAYWHITE);
    BeginMode3D(previewCamera);
    item.model.transform = MatrixScale(0.01f, 0.01f, 0.01f);
    DrawModel(item.model, (Vector3){0,0,0}, 1.0f, GRAY);
    EndMode3D();
    EndTextureMode();
}

void AssetBrowser::DrawImGuiControls() {
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
    float padding = 10.0f;
    float thumbnailSize = 128.0f;
    float cellSize = thumbnailSize + padding;
    
    float panelWidth = ImGui::GetContentRegionAvail().x;
    int columns = (int)(panelWidth / cellSize);
    if (columns < 1) columns = 1;

    if (ImGui::BeginChild("AssetGrid", ImVec2(0, 0), true)) {
        for (int i = 0; i < assets.size(); i++) {
            if (i % columns != 0)
                ImGui::SameLine();
                
            ImGui::BeginGroup();
            // Zmieniono wywołanie funkcji, usunięto drugi argument
            rlImGuiImageRenderTexture(&assets[i].thumbnail);
            ImGui::TextWrapped("%s", assets[i].name.c_str());
            ImGui::EndGroup();
        }
    }
    ImGui::EndChild();
    ImGui::PopStyleVar();
}

AssetBrowser::~AssetBrowser() {
    for (auto& item : assets) {
        UnloadModel(item.model);
        UnloadRenderTexture(item.thumbnail);
    }
}