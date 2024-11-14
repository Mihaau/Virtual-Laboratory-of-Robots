#include "imgui.h"
#include "raylib.h"
#include "raymath.h"
#include "rlImGui.h"
#include <string>
#include <vector>
#include "robotArm.h"
#include "camera.h"
#include "codeEditor.h"
#include "extras/IconsFontAwesome6.h"

#define RLIGHTS_IMPLEMENTATION
#include "rlights.h"

#if defined(PLATFORM_DESKTOP)
#define GLSL_VERSION 330
#else
#define GLSL_VERSION 100
#endif

void UpdateRenderTexture(RenderTexture2D &target, const ImVec2 &size)
{
    if (target.texture.width != (int)size.x || target.texture.height != (int)size.y)
    {
        UnloadRenderTexture(target);
        target = LoadRenderTexture((int)size.x, (int)size.y);
    }
}

int main()
{
    const int screenWidth = 1920;
    const int screenHeight = 1080;
    InitWindow(screenWidth, screenHeight, "Virtual Laboratory of Robots");
    RenderTexture2D target = LoadRenderTexture(1280, 720);

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
    robotArm.SetScale(0.01f);

    CodeEditor codeEditor;

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

        BeginTextureMode(target);
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
        EndTextureMode();

        BeginDrawing();
        ClearBackground(RAYWHITE);

        // Interfejs ImGui
        rlImGuiBegin();
        ImGui::SetNextWindowPos(ImVec2(screenWidth - 0.33 * screenWidth, 0),
                                ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(0.33 * screenWidth, screenHeight), ImGuiCond_FirstUseEver);

        ImGui::Begin("Konfiguracja", nullptr, ImGuiWindowFlags_NoMove);
        if (ImGui::BeginTabBar("OpcjeTabBar"))
        {
            // Zakładka dla kontrolek kamery
            if (ImGui::BeginTabItem("Debug"))
            {
                cameraController.DrawImGuiControls();
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
                ImGui::EndTabItem();
            }

            // Zakładka dla kontrolek ramienia robota
            if (ImGui::BeginTabItem("Edytor Kodu"))
            {
                codeEditor.Draw("##code_editor");
                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }

        ImGui::End();

// Toolbar na górze ekranu
ImGui::SetNextWindowPos(ImVec2(0, 0));
ImGui::SetNextWindowSize(ImVec2(screenWidth - 0.33 * screenWidth, 50));  // Stała wysokość 50px

ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10, 10));
ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));

ImGui::Begin("Toolbar", nullptr, 
    ImGuiWindowFlags_NoMove | 
    ImGuiWindowFlags_NoResize |
    ImGuiWindowFlags_NoTitleBar | 
    ImGuiWindowFlags_NoScrollbar);

float buttonSize = 32.0f;
ImVec2 buttonDim(120, buttonSize);

// Styl przycisków
ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.3f, 0.3f, 1.0f));
ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
// Zwiększona szerokość dla tekstu

if (ImGui::Button((std::string(ICON_FA_PLAY) + " Start").c_str(), buttonDim)) {}
if (ImGui::IsItemHovered())
    ImGui::SetTooltip("Uruchom program");
ImGui::SameLine();

if (ImGui::Button((std::string(ICON_FA_STOP) + " Stop").c_str(), buttonDim)) {}
if (ImGui::IsItemHovered())
    ImGui::SetTooltip("Zatrzymaj program");
ImGui::SameLine();

if (ImGui::Button((std::string(ICON_FA_ARROW_TREND_UP) + " Debug").c_str(), buttonDim)) {}
if (ImGui::IsItemHovered())
    ImGui::SetTooltip("Tryb debugowania");
ImGui::SameLine();

if (ImGui::Button((std::string(ICON_FA_FILE_CIRCLE_XMARK) + " Reset").c_str(), buttonDim)) {}
if (ImGui::IsItemHovered())
    ImGui::SetTooltip("Zresetuj program");
ImGui::SameLine();

if (ImGui::Button((std::string(ICON_FA_SPAGHETTI_MONSTER_FLYING) + " Config").c_str(), buttonDim)) {}
if (ImGui::IsItemHovered())
    ImGui::SetTooltip("Konfiguracja programu");

ImGui::PopStyleColor(3);
ImGui::End();
ImGui::PopStyleVar();
ImGui::PopStyleColor();

ImGui::SetNextWindowPos(ImVec2(0, 50));  // Offset o wysokość toolbara
ImGui::SetNextWindowSize(ImVec2(screenWidth - 0.33 * screenWidth, screenHeight - 50));

        ImGui::Begin("Scene View", nullptr, ImGuiWindowFlags_NoMove);
        ImVec2 contentSize = ImGui::GetContentRegionAvail();
        UpdateRenderTexture(target, contentSize);
        rlImGuiImageRenderTextureFit(&target, true);
        ImGui::End();
        rlImGuiEnd();

        EndDrawing();
    }

    // Czyszczenie zasobów
    rlImGuiShutdown();
    CloseWindow();

    return 0;
}