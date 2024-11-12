#include "imgui.h"
#include "raylib.h"
#include "raymath.h"
#include "rlImGui.h"

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

    // Zmienne kontrolujące kamerę
    float cameraX = 10.0f;
    float cameraY = 10.0f;
    float cameraZ = 10.0f;
    float cameraFOV = 45.0f;
    float zoomSpeed = 1.0f;
    float cameraDistance = 10.0f; // Początkowa odległość kamery

    Camera3D camera = {0};
    camera.position = (Vector3){cameraX, cameraY, cameraZ};
    camera.target = (Vector3){0.0f, 0.0f, 0.0f};
    camera.up = (Vector3){0.0f, 1.0f, 0.0f};
    camera.fovy = cameraFOV;
    camera.projection = CAMERA_PERSPECTIVE;

    // Ładowanie modelu
    const char *modelPath = "assets/robot.glb";
    Model model = LoadModel(modelPath);
    if (model.meshCount == 0)
    {
        TraceLog(LOG_ERROR, "Failed to load model");
        return false;
    }

    for (int i = 0; i < model.materialCount; i++)
    {
        if (&model.materials[i] != nullptr)
        {
            model.materials[i].shader = shader;
            TraceLog(LOG_INFO, "Shader applied to material %d", i);
        }float cameraPos[3]
    // Konfiguracja światła
    Light light = CreateLight(LIGHT_DIRECTIONAL, (Vector3){50.0f, 50.0f, 50.0f},
                              (Vector3){0.0f, 0.0f, 0.0f}, WHITE, shader);
    float lightIntensity = 1.0f;
    Vector3 lightPos = {50.0f, 50.0f, 50.0f};

    // Zmienne kontrolujące model
    float modelRotation = 0.0f;
    auto modelColor = RED;
    float normalizeScale = 10.0f;
    float modelScale = 0.1f / normalizeScale;

    rlImGuiSetup(true);
    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        if (float wheelMove = GetMouseWheelMove(); wheelMove != 0)
        {
            cameraDistance -= wheelMove * zoomSpeed;
            // Ograniczenie minimalnej i maksymalnej odległości
            cameraDistance = Clamp(cameraDistance, 1.0f, 100.0f);

            // Aktualizacja pozycji kamery zachowując kierunek
            Vector3 direction =
                Vector3Normalize(Vector3Subtract(camera.position, camera.target));
            camera.position =
                Vector3Add(camera.target, Vector3Scale(direction, cameraDistance));

            // Aktualizacja współrzędnych kamery
            cameraX = camera.position.x;
            cameraY = camera.position.y;
            cameraZ = camera.position.z;
        }
        // Aktualizacja kamery
        camera.position = (Vector3){cameraX, cameraY, cameraZ};
        camera.fovy = cameraFOV;

        float cameraPos[3] = {camera.position.x, camera.position.y,
                              camera.position.z};
        SetShaderValue(shader, shader.locs[SHADER_LOC_VECTOR_VIEW], cameraPos,
                       SHADER_UNIFORM_VEC3);

        BeginDrawing();
        ClearBackground(RAYWHITE);

        // Aktualizacja światła
        light.position = lightPos;

        BeginMode3D(camera);
        Vector3 position = {0.0f, 0.0f, 0.0f};
        Vector3 rotationAxis = {0.0f, 1.0f, 0.0f};

        BeginShaderMode(shader);
        UpdateLightValues(shader, light);
        DrawModelEx(model, position, rotationAxis, modelRotation,
                    (Vector3){modelScale, modelScale, modelScale}, modelColor);
        EndShaderMode();

        DrawGrid(10, 1.0f);
        DrawSphereWires(light.position, 0.5f, 8, 8, YELLOW);
        EndMode3D();

        // Interfejs ImGui
        rlImGuiBegin();
        ImGui::SetNextWindowPos(ImVec2(screenWidth - 300, 0),
                                ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(300, screenHeight), ImGuiCond_FirstUseEver);

        ImGui::Begin("Konfiguracja", nullptr, ImGuiWindowFlags_NoMove);

        // Kontrolki kamery
        if (ImGui::CollapsingHeader("Kamera"))
        {
            ImGui::SliderFloat("Kamera X", &cameraX, -100.0f, 100.0f);
            ImGui::SliderFloat("Kamera Y", &cameraY, -100.0f, 100.0f);
            ImGui::SliderFloat("Kamera Z", &cameraZ, -100.0f, 100.0f);
            ImGui::SliderFloat("FOV", &cameraFOV, 10.0f, 120.0f);
        }

        // Kontrolki modelu
        if (ImGui::CollapsingHeader("Model"))
        {
            ImGui::SliderFloat("Rotacja", &modelRotation, 0.0f, 360.0f);
            ImGui::SliderFloat("Skala", &modelScale, 0.01f, 1.0f);

            float color[4] = {
                (float)modelColor.r / 255.0f, (float)modelColor.g / 255.0f,
                (float)modelColor.b / 255.0f, (float)modelColor.a / 255.0f};

            if (ImGui::ColorEdit4("Kolor modelu", color))
            {
                modelColor = (Color){(unsigned char)(color[0] * 255.0f),
                                     (unsigned char)(color[1] * 255.0f),
                                     (unsigned char)(color[2] * 255.0f),
                                     (unsigned char)(color[3] * 255.0f)};
            }
        }

        // Kontrolki światła
        if (ImGui::CollapsingHeader("Światło"))
        {
            ImGui::SliderFloat("Intensywność", &lightIntensity, 0.0f, 2.0f);
            ImGui::SliderFloat("Światło X", &lightPos.x, -100.0f, 100.0f);
            ImGui::SliderFloat("Światło Y", &lightPos.y, -100.0f, 100.0f);
            ImGui::SliderFloat("Światło Z", &lightPos.z, -100.0f, 100.0f);

            light.color = (Color){(unsigned char)(255.0f * lightIntensity),
                                  (unsigned char)(255.0f * lightIntensity),
                                  (unsigned char)(255.0f * lightIntensity), 255};
        }

        ImGui::Text("Model: %s", modelPath);

        ImGui::End();
        rlImGuiEnd();

        EndDrawing();
    }

    // Czyszczenie zasobów
    UnloadModel(model);
    UnloadShader(shader);
    rlImGuiShutdown();
    CloseWindow();

    return 0;
}