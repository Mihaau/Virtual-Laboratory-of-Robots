#pragma once
#include "raylib.h"
#include "raymath.h"
#include "rlImGui.h"
#include "imgui.h"
#include "string"
#include "vector"

struct ArmRotation
{
    float angle;
    Vector3 axis;
};

enum class InterpolationType {
    LINEAR,
    PARABOLIC,
    SPLINE
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
    float *armLengths;
    Vector3 targetPosition;
    bool useIK;
    bool showPivotPoints;
    std::vector<Vector3> trajectoryPoints;
    bool showTrajectory = false;
        bool isAnimating = false;
    float animationTime = 0.0f;
    const float ANIMATION_DURATION = 2.0f;

    void SolveIK();
    float ClampAngle(float angle, float min, float max);
    Vector3 CalculateEndEffectorPosition();
        InterpolationType interpolationType = InterpolationType::LINEAR;
    std::vector<Vector3> controlPoints;  // Dla interpolacji spline

public:
    RobotArm(const char *modelPath, Shader shader);
    ~RobotArm();

    void Draw();
    void Update();
    void UpdateRotation(int meshIndex, float angle);
    void SetMeshVisibility(int meshIndex, bool visible);
    void SetScale(float newScale) { scale = newScale; }
    void SetColor(Color newColor) { color = newColor; }

    int GetMeshCount() const { return model.meshCount; }
    bool GetMeshVisibility(int index) const { return meshVisibility[index]; }
    float GetMeshRotation(int index) const { return meshRotations[index].angle; }
    Vector3 GetRotationAxis(int index) const { return meshRotations[index].axis; }
    void DrawPivotPoints();
    Vector3 *GetPivotPoints() { return pivotPoints; }
    void SetPivotPoint(int index, Vector3 position);
        void CalculateTrajectory();
    void DrawTrajectory();

    void DrawImGuiControls();
};