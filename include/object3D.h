#pragma once
#include "raylib.h"
#include "raymath.h"
#include "imgui.h"
#include <string>

class Object3D {
public:
    Object3D(const char* modelPath, Shader shader);
    ~Object3D();

    void Draw();
    void DrawImGuiControls();
    
    // Settery
    void SetPosition(Vector3 pos) { position = pos; }
    void SetRotation(Vector3 rot) { rotation = rot; }
    void SetScale(float scl) { scale = scl; }
    void SetColor(Color col) { color = col; }

    // Gettery
    Vector3 GetPosition() const { return position; }
    Vector3 GetRotation() const { return rotation; }
    float GetScale() const { return scale; }
    Color GetColor() const { return color; }
    Model& GetModel() { return model; }

private:
    Model model;
    Shader shader;
    Material defaultMaterial;
    
    Vector3 position;
    Vector3 rotation;
    float scale;
    Color color;
    
    int colorLoc;
    std::string modelPath;

    void UpdateTransformMatrix();
    Matrix transformMatrix;
};