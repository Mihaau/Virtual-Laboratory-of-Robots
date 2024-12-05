#include "camera.h"

CameraController::CameraController(float x, float y, float z) : cameraDistance(20.0f), zoomSpeed(1.0f), yaw(-45.0f),
                                                                pitch(-45.0f),
                                                                rotationSpeed(0.2f)
{
    camera = {0};
    camera.position = Vector3{x, y, z};
    camera.target = Vector3{0.0f, 0.0f, 0.0f};
    camera.up = Vector3{0.0f, 1.0f, 0.0f};
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    previousMousePosition = GetMousePosition();
}

void CameraController::Update()
{
    if (!isSceneViewActive)
    {
        previousMousePosition = GetMousePosition();
        return;
    }

    Vector2 currentMousePosition = GetMousePosition();
    Vector2 mouseDelta = {
        currentMousePosition.x - previousMousePosition.x,
        currentMousePosition.y - previousMousePosition.y};

    if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT) || IsKeyDown(KEY_LEFT_ALT))
    {
        // Logika obrotu kamery
        yaw += mouseDelta.x * rotationSpeed;
        pitch -= mouseDelta.y * rotationSpeed;

        pitch = Clamp(pitch, MIN_PITCH, MAX_PITCH);

        float x = cameraDistance * cosf(DEG2RAD * pitch) * cosf(DEG2RAD * yaw);
        float y = -cameraDistance * sinf(DEG2RAD * pitch);
        float z = -cameraDistance * cosf(DEG2RAD * pitch) * sinf(DEG2RAD * yaw);

        camera.position = Vector3Add(camera.target, Vector3{x, y, z});
    }
    else if (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE) || IsKeyDown(KEY_LEFT_CONTROL))
    {
        // Nowa logika przesuwania kamery
        float panSpeed = 0.1f;

        // Oblicz wektory right i up w przestrzeni świata
        Vector3 forward = Vector3Normalize(Vector3Subtract(camera.target, camera.position));
        Vector3 right = Vector3Normalize(Vector3CrossProduct(forward, camera.up));
        Vector3 up = Vector3CrossProduct(right, forward);

        // Oblicz przesunięcie
        Vector3 panRight = Vector3Scale(right, -mouseDelta.x * panSpeed);
        Vector3 panUp = Vector3Scale(up, mouseDelta.y * panSpeed);
        Vector3 totalPan = Vector3Add(panRight, panUp);

        // Przesuń zarówno pozycję kamery jak i punkt docelowy
        camera.position = Vector3Add(camera.position, totalPan);
        camera.target = Vector3Add(camera.target, totalPan);
    }

    previousMousePosition = currentMousePosition;
}

void CameraController::HandleZoom(float wheelMove)
{
    if (wheelMove != 0 && isSceneViewActive)
    {
        cameraDistance -= wheelMove * zoomSpeed;
        cameraDistance = Clamp(cameraDistance, MIN_DISTANCE, MAX_DISTANCE);

        // Aktualizacja pozycji kamery zachowując kierunek
        Vector3 direction = Vector3Normalize(Vector3Subtract(camera.position, camera.target));
        camera.position = Vector3Add(camera.target, Vector3Scale(direction, cameraDistance));
    }
}

void CameraController::DrawImGuiControls()
{
    if (ImGui::CollapsingHeader("Kamera"))
    {
        if (Vector3 pos = camera.position; ImGui::SliderFloat("Kamera X", &pos.x, -100.0f, 100.0f) ||
                                           ImGui::SliderFloat("Kamera Y", &pos.y, -100.0f, 100.0f) ||
                                           ImGui::SliderFloat("Kamera Z", &pos.z, -100.0f, 100.0f))
        {
            camera.position = pos;
        }
        if (auto mode = camera.projection; ImGui::RadioButton("Perspektywa", &mode, CAMERA_PERSPECTIVE) ||
                                           ImGui::RadioButton("Ortogonalna", &mode, CAMERA_ORTHOGRAPHIC))
        {
            camera.projection = mode;
        }

        ImGui::SliderFloat("FOV", &camera.fovy, MIN_FOV, MAX_FOV);
    }
}