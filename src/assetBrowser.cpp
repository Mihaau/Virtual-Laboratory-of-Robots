#include "assetBrowser.h"

#if defined(PLATFORM_DESKTOP)
#define GLSL_VERSION 330
#else
#define GLSL_VERSION 100
#endif

AssetBrowser::AssetBrowser() : lightController(nullptr) {
    // Inicjalizacja kamery do renderowania miniatur
    previewCamera = { 0 };
    previewCamera.position = Vector3{10.0f, 20.0f, 10.0f};
    previewCamera.target = Vector3{0.0f, 0.0f, 0.0f};
    previewCamera.up = Vector3{0.0f, 1.0f, 0.0f};
    previewCamera.fovy = 45.0f;
    previewCamera.projection = CAMERA_PERSPECTIVE;

    // Inicjalizacja shadera
    shader = LoadShader(
        TextFormat("assets/shaders/lightning.vs", GLSL_VERSION),
        TextFormat("assets/shaders/lightning.fs", GLSL_VERSION)
    );

    shader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(shader, "viewPos");
    shader.locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocation(shader, "matModel");

    if (shader.id > 0) {
        lightController = new LightController(shader);
    }

    ScanDirectory("assets/models");
}

void AssetBrowser::ScanDirectory(const std::string& path) {
    for (const auto& entry : fs::directory_iterator(path)) {
        if (entry.path().extension() == ".glb" || entry.path().extension() == ".gltf") {
            AssetItem item;
            item.name = entry.path().filename().string();
            item.path = entry.path().string();
            item.model = LoadModel(item.path.c_str());
            item.config = ModelConfig::LoadFromFile(item.path);
            
            GenerateThumbnail(item);
            assets.push_back(item);
        }
    }
}

void AssetBrowser::GenerateThumbnail(AssetItem& item, bool shouldUnloadOldTexture) {

        if (shouldUnloadOldTexture && item.thumbnail.id != 0) {
        UnloadRenderTexture(item.thumbnail);
    }

    fs::path modelDir = fs::path(item.path).parent_path() / 
                       ("." + fs::path(item.path).stem().string());
    fs::path thumbnailPath = modelDir / "thumbnail.png";

        if (fs::exists(thumbnailPath) && shouldUnloadOldTexture) {
        fs::remove(thumbnailPath);
    }

    if (fs::exists(thumbnailPath)) {
        Image img = LoadImage(thumbnailPath.string().c_str());
        item.thumbnail = LoadRenderTexture(
            item.config.thumbnail.size.width,
            item.config.thumbnail.size.height
        );
        BeginTextureMode(item.thumbnail);
            Color bgColor = {
                (unsigned char)item.config.thumbnail.background.r,
                (unsigned char)item.config.thumbnail.background.g,
                (unsigned char)item.config.thumbnail.background.b,
                (unsigned char)item.config.thumbnail.background.a
            };
            ClearBackground(bgColor);
            Texture2D tex = LoadTextureFromImage(img);
            DrawTexture(tex, 0, 0, WHITE);
            UnloadTexture(tex);
        EndTextureMode();
        UnloadImage(img);
        return;
    }

    // Użyj rozmiaru z konfiguracji
    item.thumbnail = LoadRenderTexture(
        item.config.thumbnail.size.width,
        item.config.thumbnail.size.height
    );
    
    // Ustaw kamerę z konfiguracji
    previewCamera.position = Vector3{
        item.config.thumbnail.camera.position.x,
        item.config.thumbnail.camera.position.y,
        item.config.thumbnail.camera.position.z
    };
    previewCamera.target = Vector3{
        item.config.thumbnail.camera.target.x,
        item.config.thumbnail.camera.target.y,
        item.config.thumbnail.camera.target.z
    };
    previewCamera.fovy = item.config.thumbnail.camera.fov;
    
    BeginTextureMode(item.thumbnail);
        Color bgColor = {
            (unsigned char)item.config.thumbnail.background.r,
            (unsigned char)item.config.thumbnail.background.g,
            (unsigned char)item.config.thumbnail.background.b,
            (unsigned char)item.config.thumbnail.background.a
        };
        ClearBackground(bgColor);
        BeginMode3D(previewCamera);
            
            if (lightController) {
                lightController->Update();
            }

            for(int i = 0; i < item.model.materialCount; i++) {
                item.model.materials[i].shader = shader;
            }

            // Użyj transformacji z konfiguracji
            Matrix transform = MatrixIdentity();
            transform = MatrixMultiply(transform, 
                MatrixTranslate(
                    item.config.model.position.x,
                    item.config.model.position.y,
                    item.config.model.position.z
                )
            );
            transform = MatrixMultiply(transform,
                MatrixRotateX(item.config.model.rotation.x * DEG2RAD)
            );
            transform = MatrixMultiply(transform,
                MatrixRotateY(item.config.model.rotation.y * DEG2RAD)
            );
            transform = MatrixMultiply(transform,
                MatrixRotateZ(item.config.model.rotation.z * DEG2RAD)
            );
            transform = MatrixMultiply(transform,
                MatrixScale(
                    item.config.model.scale,
                    item.config.model.scale,
                    item.config.model.scale
                )
            );
            
            item.model.transform = transform;
            DrawModel(item.model, Vector3Zero(), 1.0f, WHITE);
            
        EndMode3D();
    EndTextureMode();

    fs::create_directories(modelDir);
    Image img = LoadImageFromTexture(item.thumbnail.texture);
    ImageFlipVertical(&img);
    ExportImage(img, thumbnailPath.string().c_str());
    UnloadImage(img);
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
            if (i % columns != 0) {
                ImGui::SameLine();
            }

        ImGui::BeginGroup();
        ImGui::PushID(i);
        
        // Dodaj detekcję prawego przycisku myszy
        rlImGuiImageRenderTexture(&assets[i].thumbnail);
        if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(1)) { // 1 = prawy przycisk
            ImGui::OpenPopup("asset_context_menu");
            selectedItem = &assets[i];
        }

        // Menu kontekstowe
        if (ImGui::BeginPopup("asset_context_menu")) {
            if (ImGui::MenuItem("Dodaj do sceny")) {
                // TODO: Implementacja dodawania modelu do sceny
                ImGui::CloseCurrentPopup();
            }
            if (ImGui::MenuItem("Edytuj konfigurację")) {
                showConfigEditor = true;
                editingConfig = selectedItem->config;
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        ImGui::TextWrapped("%s", assets[i].name.c_str());
        ImGui::PopID();
        ImGui::EndGroup();
        }
    }
    ImGui::EndChild();
    ImGui::PopStyleVar();


    // Okno edycji konfiguracji
    if (showConfigEditor && selectedItem) {
        ImGui::SetNextWindowSize(ImVec2(400, 400), ImGuiCond_FirstUseEver);
        if (ImGui::Begin("Edycja konfiguracji", &showConfigEditor)) {
            ImGui::Text("Model: %s", selectedItem->name.c_str());
            
            if (ImGui::CollapsingHeader("Model")) {
                ImGui::DragFloat("Skala", &editingConfig.model.scale, 0.01f, 0.01f, 10.0f);
                ImGui::DragFloat3("Rotacja", &editingConfig.model.rotation.x, 1.0f, -360.0f, 360.0f);
                ImGui::DragFloat3("Pozycja", &editingConfig.model.position.x, 0.1f);
            }

if (ImGui::CollapsingHeader("Miniatura")) {
    ImGui::ColorEdit4("Tło", &editingConfig.thumbnail.background.r);
    ImGui::DragFloat2("Rozmiar", (float*)&editingConfig.thumbnail.size.width, 1.0f, 32.0f, 512.0f);
    
                
                if (ImGui::TreeNode("Kamera")) {
                    ImGui::DragFloat3("Pozycja", &editingConfig.thumbnail.camera.position.x, 0.1f);
                    ImGui::DragFloat3("Cel", &editingConfig.thumbnail.camera.target.x, 0.1f);
                    ImGui::DragFloat("FOV", &editingConfig.thumbnail.camera.fov, 1.0f, 1.0f, 120.0f);
                    ImGui::TreePop();
                }
            }

            if (ImGui::Button("Zapisz")) {
                selectedItem->config = editingConfig;
                editingConfig.SaveToFile(selectedItem->path);
                GenerateThumbnail(*selectedItem, true);
                showConfigEditor = false;
            }
            ImGui::SameLine();
            if (ImGui::Button("Anuluj")) {
                showConfigEditor = false;
            }
        }
        ImGui::End();
    }


}

AssetBrowser::~AssetBrowser() {
    for (auto& item : assets) {
        UnloadModel(item.model);
        UnloadRenderTexture(item.thumbnail);
    }
    
    if (shader.id > 0) {
        UnloadShader(shader);
    }
    
    if (lightController) {
        delete lightController;
        lightController = nullptr;
    }
}