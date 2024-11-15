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
#include "assetBrowser.h"
#include "toolbar.h"
#include "lightController.h"
#include "object3D.h"
#include "logWindow.h"
#include "imgui_theme.h"

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
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(screenWidth, screenHeight, "Virtual Laboratory of Robots");
    SetWindowMinSize(1280, 720);
    Image icon = LoadImage("assets/images/icon.png");
    SetWindowIcon(icon);
    UnloadImage(icon);
    RenderTexture2D target = LoadRenderTexture(1280, 720);

    // Inicjalizacja shadera oświetlenia
    Shader shader =
        LoadShader(TextFormat("assets/shaders/lightning.vs", GLSL_VERSION),
                   TextFormat("assets/shaders/lightning.fs", GLSL_VERSION));

    CameraController cameraController(10.0f, 10.0f, 10.0f);

    // Inicjalizacja ramienia robota
    RobotArm robotArm("assets/models/robot.glb", shader);
    robotArm.SetScale(0.005f);

    Object3D cat("assets/models/cat.glb", shader);
    cat.SetScale(0.01f);
    cat.SetPosition(Vector3{5.0f, 0.0f, 2.0f});

    CodeEditor codeEditor;
    AssetBrowser assetBrowser;
    ToolBar toolBar(screenWidth);
    LogWindow logWindow;

    shader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(shader, "viewPos");
    shader.locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocation(shader, "matModel");
    LightController lightController(shader);

    rlImGuiSetup(true);
        ImGui::Options options;
    options.ApplyTheme();
    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        const float currentWidth = (float)GetScreenWidth();
        const float currentHeight = (float)GetScreenHeight();
        const float sidebarWidth = currentWidth * 0.4; // 33% of window width
        const float toolbarHeight = 50.0f;
        const float logWindowHeight = 200.0f; // Wysokość okna logów

        if (float wheelMove = GetMouseWheelMove(); wheelMove != 0)
        {
            cameraController.HandleZoom(wheelMove);
        }
        cameraController.Update();
        //////////////////////////////////////////////////////////////////////////////////////////
        BeginTextureMode(target);
        ClearBackground(RAYWHITE);

        lightController.Update();
        BeginMode3D(cameraController.GetCamera());

        robotArm.Draw();
        cat.Draw();
        DrawGrid(10, 1.0f);
        DrawSphereWires(lightController.GetLight().position, 0.5f, 8, 8, YELLOW);
        robotArm.DrawPivotPoints();

        EndMode3D();
        EndTextureMode();
        ///////////////////////////////////////////////////////////////////////////////////////////
        BeginDrawing();
        ClearBackground(DARKGRAY);

        toolBar.UpdateScreenWidth(currentWidth);

        // Interfejs ImGui
        rlImGuiBegin();
        ImGui::SetNextWindowPos(ImVec2(currentWidth - sidebarWidth, toolbarHeight));
        ImGui::SetNextWindowSize(ImVec2(sidebarWidth, currentHeight - toolbarHeight));

        ImGui::Begin("Konfiguracja", nullptr, ImGuiWindowFlags_NoMove);
        if (ImGui::BeginTabBar("OpcjeTabBar"))
        {
            // Zakładka dla kontrolek kamery
            if (ImGui::BeginTabItem("Debug"))
            {
                cameraController.DrawImGuiControls();
                robotArm.DrawImGuiControls();
                cat.DrawImGuiControls();
                lightController.DrawImGuiControls();

                ImGui::Text("Model: %s", "assets/robot.glb");

                // Dodanie przycisków do wysyłania testowych wiadomości logów
                if (ImGui::Button("Dodaj log INFO"))
                {
                    logWindow.AddLog("To jest testowa wiadomość INFO", LogLevel::Info);
                }
                if (ImGui::Button("Dodaj log WARN"))
                {
                    logWindow.AddLog("To jest testowa wiadomość WARN", LogLevel::Warning);
                }
                if (ImGui::Button("Dodaj log ERROR"))
                {
                    logWindow.AddLog("To jest testowa wiadomość ERROR", LogLevel::Error);
                }
                ImGui::EndTabItem();
            }

            // Zakładka dla kontrolek ramienia robota
            if (ImGui::BeginTabItem("Edytor Kodu"))
            {
                codeEditor.Draw("##code_editor");
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Assets"))
            {
                assetBrowser.DrawImGuiControls();
                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }

        ImGui::End();

        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2(currentWidth, toolbarHeight));
        toolBar.Draw();

        ImGui::SetNextWindowPos(ImVec2(0, toolbarHeight));
        ImGui::SetNextWindowSize(ImVec2(currentWidth - sidebarWidth, currentHeight - toolbarHeight - logWindowHeight));

        ImGui::Begin("Scene View", nullptr, ImGuiWindowFlags_NoMove);
        ImVec2 contentSize = ImGui::GetContentRegionAvail();
        UpdateRenderTexture(target, contentSize);
        rlImGuiImageRenderTextureFit(&target, true);
        cameraController.SetSceneViewActive(ImGui::IsWindowHovered());
        ImGui::End();

        ImGui::SetNextWindowPos(ImVec2(0, currentHeight - logWindowHeight));
        ImGui::SetNextWindowSize(ImVec2(currentWidth - sidebarWidth, logWindowHeight));
        logWindow.Draw("Logs",
                       ImVec2(0, currentHeight - logWindowHeight),
                       ImVec2(currentWidth - sidebarWidth, logWindowHeight));
        rlImGuiEnd();

        EndDrawing();
    }

    // Czyszczenie zasobów
    rlImGuiShutdown();
    CloseWindow();

    return 0;
}