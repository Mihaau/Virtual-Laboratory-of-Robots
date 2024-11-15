#include "modelConfig.h"

fs::path ModelConfig::GetConfigPath(const std::string& modelPath) {
    fs::path modelDir = fs::path(modelPath).parent_path() / 
                       ("." + fs::path(modelPath).stem().string());
    return modelDir / "config.json";
}

void ModelConfig::LoadDefaults() {
    model.scale = 0.01f;
    model.rotation = {0.0f, 0.0f, 0.0f};
    model.position = {0.0f, 0.0f, 0.0f};

    thumbnail.background = {255.0f, 255.0f, 255.0f, 255.0f};
    thumbnail.size = {128, 128};
    thumbnail.camera.position = {10.0f, 10.0f, 10.0f};
    thumbnail.camera.target = {0.0f, 5.0f, 0.0f};
    thumbnail.camera.fov = 45.0f;

    materials.ambient = {0.2f, 0.2f, 0.2f, 1.0f};
}

ModelConfig ModelConfig::LoadFromFile(const std::string& modelPath) {
    ModelConfig config;
    config.LoadDefaults();

    fs::path configPath = GetConfigPath(modelPath);
    if (!fs::exists(configPath)) {
        config.SaveToFile(modelPath);
        return config;
    }

    try {
        std::ifstream file(configPath);
        json j;
        file >> j;

        // Model
        config.model.scale = j["model"]["scale"];
        config.model.rotation.x = j["model"]["rotation"]["x"];
        config.model.rotation.y = j["model"]["rotation"]["y"];
        config.model.rotation.z = j["model"]["rotation"]["z"];
        config.model.position.x = j["model"]["position"]["x"];
        config.model.position.y = j["model"]["position"]["y"];
        config.model.position.z = j["model"]["position"]["z"];

        // Thumbnail
        config.thumbnail.background.r = j["thumbnail"]["background"]["r"];
        config.thumbnail.background.g = j["thumbnail"]["background"]["g"];
        config.thumbnail.background.b = j["thumbnail"]["background"]["b"];
        config.thumbnail.background.a = j["thumbnail"]["background"]["a"];
        
        config.thumbnail.size.width = j["thumbnail"]["size"]["width"];
        config.thumbnail.size.height = j["thumbnail"]["size"]["height"];
        
        config.thumbnail.camera.position.x = j["thumbnail"]["camera"]["position"]["x"];
        config.thumbnail.camera.position.y = j["thumbnail"]["camera"]["position"]["y"];
        config.thumbnail.camera.position.z = j["thumbnail"]["camera"]["position"]["z"];
        
        config.thumbnail.camera.target.x = j["thumbnail"]["camera"]["target"]["x"];
        config.thumbnail.camera.target.y = j["thumbnail"]["camera"]["target"]["y"];
        config.thumbnail.camera.target.z = j["thumbnail"]["camera"]["target"]["z"];
        
        config.thumbnail.camera.fov = j["thumbnail"]["camera"]["fov"];

        // Materials
        config.materials.ambient.r = j["materials"]["ambient"]["r"];
        config.materials.ambient.g = j["materials"]["ambient"]["g"];
        config.materials.ambient.b = j["materials"]["ambient"]["b"];
        config.materials.ambient.a = j["materials"]["ambient"]["a"];

    } catch (const json::exception& e) {
        TraceLog(LOG_ERROR, "Błąd wczytywania konfiguracji: %s", e.what());
    }

    return config;
}

void ModelConfig::SaveToFile(const std::string& modelPath) {
    fs::path configPath = GetConfigPath(modelPath);
    fs::create_directories(configPath.parent_path());

    json j = {
        {"model", {
            {"scale", model.scale},
            {"rotation", {
                {"x", model.rotation.x},
                {"y", model.rotation.y},
                {"z", model.rotation.z}
            }},
            {"position", {
                {"x", model.position.x},
                {"y", model.position.y},
                {"z", model.position.z}
            }}
        }},
        {"thumbnail", {
            {"background", {
                {"r", thumbnail.background.r},
                {"g", thumbnail.background.g},
                {"b", thumbnail.background.b},
                {"a", thumbnail.background.a}
            }},
            {"size", {
                {"width", thumbnail.size.width},
                {"height", thumbnail.size.height}
            }},
            {"camera", {
                {"position", {
                    {"x", thumbnail.camera.position.x},
                    {"y", thumbnail.camera.position.y},
                    {"z", thumbnail.camera.position.z}
                }},
                {"target", {
                    {"x", thumbnail.camera.target.x},
                    {"y", thumbnail.camera.target.y},
                    {"z", thumbnail.camera.target.z}
                }},
                {"fov", thumbnail.camera.fov}
            }}
        }},
        {"materials", {
            {"ambient", {
                {"r", materials.ambient.r},
                {"g", materials.ambient.g},
                {"b", materials.ambient.b},
                {"a", materials.ambient.a}
            }}
        }}
    };

    std::ofstream file(configPath);
    file << std::setw(4) << j << std::endl;
}