#pragma once
#include "raylib.h"
#include "raymath.h"
#include "rlImGui.h"
#include "imgui.h"
#include "string"
#include "logWindow.h"
#include "lua.hpp"
#include "vector"
#include "robotKinematics.h"

class RobotArm {
private:
    Model model;
    bool* meshVisibility;
    ArmRotation* meshRotations;
    float scale;
    Color color;
    Shader shader;
    int colorLoc;
    Material defaultMaterial;
    Vector3* pivotPoints;
    float* armLengths;
    bool showPivotPoints;
    bool showTrajectory;
    bool isAnimating;
    float animationTime;
    const float ANIMATION_DURATION = 2.0f;
    
    RobotKinematics* kinematics;
    bool stepMode;
    int currentLine;
    lua_State* L;
public:
    RobotArm(const char* modelPath, Shader shader);
    ~RobotArm();

    void Draw();
    void Update();
    void UpdateRotation(int meshIndex, float angle);
    void SetMeshVisibility(int meshIndex, bool visible);
    void SetScale(float newScale);
    void SetColor(Color newColor) { color = newColor; }

    int GetMeshCount() const { return model.meshCount; }
    bool GetMeshVisibility(int index) const { return meshVisibility[index]; }
    float GetMeshRotation(int index) const { return meshRotations[index].angle; }
    Vector3 GetRotationAxis(int index) const { return meshRotations[index].axis; }
    void DrawPivotPoints();
    Vector3* GetPivotPoints() { return pivotPoints; }
    void SetPivotPoint(int index, Vector3 position);
    void DrawTrajectory();
    void DrawImGuiControls();
    
    void MoveToPosition(const Vector3& position);
    void RotateJoint(int jointIndex, float angle);
    void Step();
    void SetStepMode(bool enabled);
    int GetCurrentLine() const;
    void ExecuteLuaScript(const std::string& script);
};