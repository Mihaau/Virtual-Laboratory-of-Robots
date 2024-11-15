#ifndef CAMERA_H
#define CAMERA_H

#include "raylib.h"
#include "raymath.h"
#include "imgui.h"

class CameraController {
public:
    CameraController(float x = 10.0f, float y = 10.0f, float z = 10.0f);
    
    void Update();
    void DrawImGuiControls();
    void HandleZoom(float wheelMove);
    void SetSceneViewActive(bool active) { isSceneViewActive = active; }
    
    Camera3D& GetCamera() { return camera; }
    const Vector3& GetPosition() const { return camera.position; }
    float GetFOV() const { return camera.fovy; }

private:
    Camera3D camera;
    float cameraDistance;
    float zoomSpeed;

    float yaw;
    float pitch;
    Vector2 previousMousePosition;
    float rotationSpeed;

    bool isSceneViewActive = false;
    
    // Limity kamery
    static constexpr float MIN_DISTANCE = 1.0f;
    static constexpr float MAX_DISTANCE = 100.0f;
    static constexpr float MIN_FOV = 10.0f;
    static constexpr float MAX_FOV = 120.0f;
        static constexpr float MIN_PITCH = -89.0f;
    static constexpr float MAX_PITCH = 89.0f;
};

#endif // CAMERA_H