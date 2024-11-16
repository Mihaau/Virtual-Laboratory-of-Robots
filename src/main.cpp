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
#include "extras/FA6FreeSolidFontData.h"
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

void DrawSplashScreen(bool &showSplashScreen, Texture2D &logo)
{
    const ImGuiViewport *viewport = ImGui::GetMainViewport();
    ImVec2 center = viewport->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(600, 400));

    if (ImGui::Begin("Splash Screen", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar))
    {
        ImVec2 windowSize = ImGui::GetWindowSize();
        ImVec2 imageSize = ImVec2(int(2979 / 5.5), int(625 / 5.5));
        ImVec2 imagePos = ImVec2((windowSize.x - imageSize.x) * 0.5f, ImGui::GetCursorPosY());
        ImGui::SetCursorPos(imagePos);
        rlImGuiImageSize(&logo, imageSize.x, imageSize.y);
        ImGui::Separator();
        ImGui::TextWrapped("Welcome to the Virtual Laboratory of Robots. This project allows you to simulate and control various robotic arms and other 3D objects in a virtual environment.");
        ImGui::Separator();
        ImGui::Text("Click to continue...");

        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
        {
            showSplashScreen = false;
        }

        ImGui::End();
    }
}

int main()
{
    const int screenWidth = 1920;
    const int screenHeight = 1080;
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT);
    InitWindow(screenWidth, screenHeight, "Virtual Laboratory of Robots");
    SetWindowMinSize(1280, 720);
    Image icon = LoadImage("assets/images/icon.png");
    SetWindowIcon(icon);
    UnloadImage(icon);
    RenderTexture2D target = LoadRenderTexture(1920, 1080);
    bool showSplashScreen = true;
    Texture2D logo = LoadTexture("assets/images/banner.png");
    std::vector<Object3D *> sceneObjects;

    // Inicjalizacja shadera oświetlenia
    Shader shader =
        LoadShader(TextFormat("assets/shaders/lightning.vs", GLSL_VERSION),
                   TextFormat("assets/shaders/lightning.fs", GLSL_VERSION));

    CameraController cameraController(10.0f, 10.0f, 10.0f);

    // Inicjalizacja ramienia robota
    RobotArm robotArm("assets/models/robot.glb", shader);
    robotArm.SetScale(0.005f);

    // Object3D cat("assets/models/cat.glb", shader);
    // cat.SetScale(0.01f);
    // cat.SetPosition(Vector3{5.0f, 0.0f, 2.0f});

    CodeEditor codeEditor;
    AssetBrowser assetBrowser;
    assetBrowser.onAddObjectToScene = [&shader, &sceneObjects](const char *modelPath)
    {
        Object3D *obj = Object3D::Create(modelPath, shader);
        sceneObjects.push_back(obj);
    };

    ToolBar toolBar(screenWidth);
    LogWindow logWindow;

    shader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(shader, "viewPos");
    shader.locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocation(shader, "matModel");
    LightController lightController(shader);

    rlImGuiSetup(true);

    // Załaduj czcionkę Roboto z odpowiednim zakresem znaków
    ImGuiIO &io = ImGui::GetIO();
    ImFontConfig fontConfig;
    fontConfig.OversampleH = 3;
    fontConfig.OversampleV = 1;
    static const ImWchar ranges[] = {
        0x0020,
        0x00FF, // Podstawowy łaciński + rozszerzony łaciński
        0x0100,
        0x017F, // Rozszerzony łaciński-A
        0x0180,
        0x024F, // Rozszerzony łaciński-B
        0x0250,
        0x02AF, // IPA rozszerzony
        0x2C60,
        0x2C7F, // Łaciński rozszerzony-C
        0xA720,
        0xA7FF, // Łaciński rozszerzony-D
        0,
    };
    ImFont *roboto = io.Fonts->AddFontFromFileTTF("assets/fonts/Roboto-Bold.ttf", 16.0f, &fontConfig, ranges);

    static const ImWchar icons_ranges[] = {ICON_MIN_FA, ICON_MAX_FA, 0};
    ImFontConfig icons_config;
    icons_config.MergeMode = true;
    icons_config.PixelSnapH = true;
    io.Fonts->AddFontFromMemoryCompressedTTF(fa_solid_900_compressed_data, fa_solid_900_compressed_size, 16.0f, &icons_config, icons_ranges);

    io.FontDefault = roboto;
    io.Fonts->Build();
    rlImGuiReloadFonts();

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
        ClearBackground(DARKGRAY);

        lightController.Update();
        BeginMode3D(cameraController.GetCamera());

        robotArm.Draw();
        for (auto *obj : sceneObjects)
        {
            obj->Draw();
        }
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

        ImGui::Begin("Konfiguracja", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
        if (ImGui::BeginTabBar("OpcjeTabBar"))
        {
            // Zakładka dla kontrolek kamery
            if (ImGui::BeginTabItem("Debug"))
            {
                cameraController.DrawImGuiControls();
                robotArm.DrawImGuiControls();

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
            if (ImGui::BeginTabItem("Obiekty"))
            {
                for (auto *obj : sceneObjects)
                {
                    obj->DrawImGuiControls();
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

        ImGui::Begin("Scene View", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
        ImVec2 contentSize = ImGui::GetContentRegionAvail();
        UpdateRenderTexture(target, contentSize);
        SetTextureFilter(target.texture, TEXTURE_FILTER_TRILINEAR);
        rlImGuiImageRenderTextureFit(&target, true);
        cameraController.SetSceneViewActive(ImGui::IsWindowHovered());
        ImGui::End();

        ImGui::SetNextWindowPos(ImVec2(0, currentHeight - logWindowHeight));
        ImGui::SetNextWindowSize(ImVec2(currentWidth - sidebarWidth, logWindowHeight));
        logWindow.Draw("Logs",
                       ImVec2(0, currentHeight - logWindowHeight),
                       ImVec2(currentWidth - sidebarWidth, logWindowHeight));
        if (showSplashScreen)
        {
            DrawSplashScreen(showSplashScreen, logo);
        }
        rlImGuiEnd();

        EndDrawing();
    }
    for (auto *obj : sceneObjects)
    {
        delete obj;
    }
    sceneObjects.clear();

    // Czyszczenie zasobów
    UnloadRenderTexture(target);
    UnloadShader(shader);
    rlImGuiShutdown();
    CloseWindow();

    return 0;
}