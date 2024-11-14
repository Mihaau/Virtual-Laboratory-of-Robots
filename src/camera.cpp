#include "camera.h"

CameraController::CameraController(float x, float y, float z) : cameraDistance(10.0f), zoomSpeed(1.0f) {
    camera = { 0 };
    camera.position = (Vector3){ x, y, z };
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;
}

void CameraController::Update() {
    // Aktualizacja kamery może być rozszerzona w przyszłości
}

void CameraController::HandleZoom(float wheelMove) {
    if (wheelMove != 0) {
        cameraDistance -= wheelMove * zoomSpeed;
        cameraDistance = Clamp(cameraDistance, MIN_DISTANCE, MAX_DISTANCE);

        // Aktualizacja pozycji kamery zachowując kierunek
        Vector3 direction = Vector3Normalize(Vector3Subtract(camera.position, camera.target));
        camera.position = Vector3Add(camera.target, Vector3Scale(direction, cameraDistance));
    }
}

void CameraController::DrawImGuiControls() {
    if (ImGui::CollapsingHeader("Kamera")) {
        Vector3 pos = camera.position;
        if (ImGui::SliderFloat("Kamera X", &pos.x, -100.0f, 100.0f) ||
            ImGui::SliderFloat("Kamera Y", &pos.y, -100.0f, 100.0f) ||
            ImGui::SliderFloat("Kamera Z", &pos.z, -100.0f, 100.0f)) {
            camera.position = pos;
        }
        
        ImGui::SliderFloat("FOV", &camera.fovy, MIN_FOV, MAX_FOV);
    }
}