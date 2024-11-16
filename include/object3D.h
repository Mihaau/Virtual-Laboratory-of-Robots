#pragma once
#include "raylib.h"
#include "raymath.h"
#include "imgui.h"
#include <string>
#include <filesystem>
namespace fs = std::filesystem;

class Object3D
{
public:
    Object3D(const char *modelPath, Shader shader);
    ~Object3D();

    void Draw();
    void DrawImGuiControls();
    static Object3D *Create(const char *modelPath, Shader shader);
    static void ResetIdCounter() { nextId = 0; }

    // Settery
    void SetPosition(Vector3 pos) { position = pos; }
    void SetRotation(Vector3 rot) { rotation = rot; }
    void SetScale(float scl) { scale = scl; }
    void SetColor(Color col) { color = col; }

    // Gettery
    int GetId() const { return id; }
    Vector3 GetPosition() const { return position; }
    Vector3 GetRotation() const { return rotation; }
    float GetScale() const { return scale; }
    Color GetColor() const { return color; }
    Model &GetModel() { return model; }

private:
    Model model;
    Shader shader;
    Material defaultMaterial;

    static int nextId;
    const int id;  // zmień na const
    std::string displayName;

    Vector3 position;
    void SetUIPosition(const Vector3& uiPos) {
        position = Vector3{
            uiPos.x / scale,
            uiPos.y / scale,
            uiPos.z / scale
        };
    }
    Vector3 rotation;
    float scale;
    Color color;

    int colorLoc;
    std::string modelPath;

    void UpdateTransformMatrix();
    Matrix transformMatrix;
};