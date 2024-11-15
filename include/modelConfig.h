#pragma once
#include <nlohmann/json.hpp>
#include <filesystem>
#include <fstream>
#include "raylib.h"
#include "raymath.h"

using json = nlohmann::json;
namespace fs = std::filesystem;

struct Vector3Config {
    float x{0.0f};
    float y{0.0f};
    float z{0.0f};
};

struct ColorConfig {
    float r{0.0f};
    float g{0.0f};
    float b{0.0f};
    float a{1.0f};
};

struct SizeConfig {
    int width{128};
    int height{128};
};

struct ModelConfig {
public:
    struct {
        float scale{0.01f};
        Vector3Config rotation;
        Vector3Config position;
    } model;

    struct {
        ColorConfig background;
        SizeConfig size;
        struct {
            Vector3Config position;
            Vector3Config target;
            float fov{45.0f};
        } camera;
    } thumbnail;

    struct {
        ColorConfig ambient;
    } materials;

    static ModelConfig LoadFromFile(const std::string& modelPath);
    void SaveToFile(const std::string& modelPath);

private:
    static fs::path GetConfigPath(const std::string& modelPath);
    void LoadDefaults();
};