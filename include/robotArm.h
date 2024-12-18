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
#include "object3D.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

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

        Vector3 gripperPosition;    // Position of the gripper sphere
    float gripperRadius;        // Radius of the gripper sphere
    bool isColliding;          // Collision state
    Color gripperColor;

    Object3D* grippedObject = nullptr;
    bool isGripping = false;
    Vector3 gripOffset; // Offset między chwytakiem a obiektem
    Vector3 rotationOffset; // Offset między rotacją chwytaka a obiektem

        LogWindow& logWindow;
    const std::vector<Object3D*>* sceneObjects = nullptr;
    Vector3 eulerAnglesObject;
    Vector3 baseDirection;
    Vector3 baseUpDirection;
    Vector3 baseSideDirection;

    Vector3 rotatedPoint;

        bool isTracing = false;
    std::vector<Vector3> tracePath;
    Vector3 originalGripOffset;
    int meshCount;
public:
    RobotArm(const char* modelPath, const char* configPath, Shader shader);
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

        void CheckCollisions(const std::vector<Object3D*>& objects);
    void DrawGripper();

        void GripObject();
    void ReleaseObject();
    bool CanGrip() const { return isColliding && !isGripping; }
    bool IsGripping() const { return isGripping; }
void SetSceneObjects(const std::vector<Object3D*>* objects) { sceneObjects = objects; }
    void DrawGripperDirection();
    float GetEndEffectorRoll();
    void MoveTo(Vector3 target, InterpolationType type);

        void StartTracing();
    void StopTracing();
    void ClearTrace(); 
    void Reset();
};