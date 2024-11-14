#include "imgui.h"
#include "raylib.h"
#include "raymath.h"
#include "rlImGui.h"
#include <string>
#include <vector>
#include "robotArm.h"
#include "camera.h"

#define RLIGHTS_IMPLEMENTATION
#include "rlights.h"

#if defined(PLATFORM_DESKTOP)
#define GLSL_VERSION 330
#else
#define GLSL_VERSION 100
#endif

int main()
{
    const int screenWidth = 1920;
    const int screenHeight = 1080;
    InitWindow(screenWidth, screenHeight, "Virtual Laboratory of Robots");

    // Inicjalizacja shadera oświetlenia
    Shader shader =
        LoadShader(TextFormat("assets/shaders/lightning.vs", GLSL_VERSION),
                   TextFormat("assets/shaders/lightning.fs", GLSL_VERSION));

    shader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(shader, "viewPos");
    shader.locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocation(shader, "matModel");

    // Ambient light
    int ambientLoc = GetShaderLocation(shader, "ambient");
    SetShaderValue(shader, ambientLoc, (float[4]){0.2f, 0.2f, 0.2f, 1.0f},
                   SHADER_UNIFORM_VEC4);

    CameraController cameraController(10.0f, 10.0f, 10.0f);

    // Inicjalizacja ramienia robota
    RobotArm robotArm("assets/models/robot.glb", shader);
    robotArm.SetScale(0.1f);

    // Konfiguracja światła
    Light light = CreateLight(LIGHT_DIRECTIONAL, (Vector3){50.0f, 50.0f, 50.0f}, (Vector3){0.0f, 0.0f, 0.0f}, WHITE, shader);
    float lightIntensity = 1.0f;
    Vector3 lightPos = {50.0f, 50.0f, 50.0f};

    rlImGuiSetup(true);
    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        if (float wheelMove = GetMouseWheelMove(); wheelMove != 0)
        {
            cameraController.HandleZoom(wheelMove);
        }
        cameraController.Update();

        BeginDrawing();
        ClearBackground(RAYWHITE);

        // Aktualizacja światła
        light.position = lightPos;
        UpdateLightValues(shader, light);

        BeginMode3D(cameraController.GetCamera());

        robotArm.Draw();

        DrawGrid(10, 1.0f);
        DrawSphereWires(light.position, 0.5f, 8, 8, YELLOW);
        robotArm.DrawPivotPoints();
        EndMode3D();

        // Interfejs ImGui
        rlImGuiBegin();
        ImGui::SetNextWindowPos(ImVec2(screenWidth - 300, 0),
                                ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(300, screenHeight), ImGuiCond_FirstUseEver);

        ImGui::Begin("Konfiguracja", nullptr, ImGuiWindowFlags_NoMove);

        cameraController.DrawImGuiControls();
        // Kontrolki ramienia robota
        robotArm.DrawImGuiControls();

        // Kontrolki światła
        if (ImGui::CollapsingHeader("Światło"))
        {
            ImGui::SliderFloat("Intensywność", &lightIntensity, 0.0f, 1.0f);
            ImGui::SliderFloat("Światło X", &lightPos.x, -100.0f, 100.0f);
            ImGui::SliderFloat("Światło Y", &lightPos.y, -100.0f, 100.0f);
            ImGui::SliderFloat("Światło Z", &lightPos.z, -100.0f, 100.0f);

            light.color = (Color){(unsigned char)(255.0f * lightIntensity),
                                  (unsigned char)(255.0f * lightIntensity),
                                  (unsigned char)(255.0f * lightIntensity), 255};
        }

        ImGui::Text("Model: %s", "assets/robot.glb");

        ImGui::End();
        rlImGuiEnd();

        EndDrawing();
    }

    // Czyszczenie zasobów
    rlImGuiShutdown();
    CloseWindow();

    return 0;
}