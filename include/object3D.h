#pragma once
#include "raylib.h"
#include "raymath.h"
#include "imgui.h"
#include <string>
#include <filesystem>
#include <vector>
namespace fs = std::filesystem;

class Object3D
{
public:
    Object3D(const char* modelPath, Shader shader, float initialScale = 1.0f);
    ~Object3D();

    void Draw();
    void DrawImGuiControls();
    static Object3D* Create(const char* modelPath, Shader shader, float initialScale = 1.0f);
    static void ResetIdCounter() { nextId = 0; }

    // Settery
    void SetPosition(Vector3 pos)
    {
        position = pos;
        UpdateTransformMatrix();
    }
    void SetRotation(Vector3 rot)
    {
        rotation = rot;
        UpdateTransformMatrix();
    }
    void SetGlobalRotation(Vector3 newRot)

    {
        // Resetujemy aktualną rotację do zera
        rotation = Vector3Zero();
        UpdateTransformMatrix();

        // Ustawiamy nową rotację bezwzględną
        rotation = newRot;
        UpdateTransformMatrix();
    }

    void SetScale(float scl)
    {
        scale = scl;
        UpdateTransformMatrix();
    }
    void SetColor(Color col) { color = col; }

    void SetTransformMatrix(const Matrix& matrix);

    // Gettery
    int GetId() const { return id; }
    Vector3 GetPosition() const { return position; }
    Vector3 GetRotation() const { return rotation; }
    float GetScale() const { return scale; }
    Color GetColor() const { return color; }
    const Model &GetModel() const { return model; }
    static void Delete(Object3D *obj);
    bool markedForDeletion = false;
    static std::vector<Object3D *> deleteQueue;
    static void ProcessDeleteQueue();
    const std::string &GetModelPath() const { return modelPath; }
    Matrix GetTransform() const { return transformMatrix; }
    void DrawObjectBoundingBox();

private:
    Model model;
    Shader shader;
    Material defaultMaterial;
    Material material;

    static int nextId;
    const int id; // zmień na const
    std::string displayName;

    Vector3 position;
    void SetUIPosition(const Vector3 &uiPos)
    {
        position = Vector3{
            uiPos.x / scale,
            uiPos.y / scale,
            uiPos.z / scale};
    }
    Vector3 rotation;
    float scale;
    Color color;

    int colorLoc;
    std::string modelPath;

    void UpdateTransformMatrix();
    Matrix transformMatrix;
    bool showBoundingBox = false;
};