#pragma once
#include "raylib.h"
#include "raymath.h"
#include "rlImGui.h"
#include "imgui.h"
#include "string"

struct ArmRotation
{
    float angle;
    Vector3 axis;
};

class RobotArm
{
private:
    Model model;
    bool *meshVisibility;
    ArmRotation *meshRotations;
    float scale;
    Color color;
    Shader shader;
    int colorLoc;
    Material defaultMaterial;
    Vector3 *pivotPoints;
    bool showPivotPoints;

public:
    RobotArm(const char *modelPath, Shader shader);
    ~RobotArm();

    void Draw();
    void UpdateRotation(int meshIndex, float angle);
    void SetMeshVisibility(int meshIndex, bool visible);
    void SetScale(float newScale) { scale = newScale; }
    void SetColor(Color newColor) { color = newColor; }

    int GetMeshCount() const { return model.meshCount; }
    bool GetMeshVisibility(int index) const { return meshVisibility[index]; }
    float GetMeshRotation(int index) const { return meshRotations[index].angle; }
    Vector3 GetRotationAxis(int index) const { return meshRotations[index].axis; }
    void DrawPivotPoints(bool showPivots = true);
    Vector3 *GetPivotPoints() { return pivotPoints; }
    void SetPivotPoint(int index, Vector3 position);

    void DrawImGuiControls();
};