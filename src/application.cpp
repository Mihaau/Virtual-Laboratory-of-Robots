#include "application.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "imgui_internal.h"

void UseImGui::Init(GLFWwindow** window) {

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(*window, true);
    ImGui_ImplOpenGL3_Init("#version 460");

}

void UseImGui::NewFrame () {
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

}

void UseImGui::Update () {

    static bool opt_padding = false; // Is there padding (a blank space) between the window edge and the Dockspace?
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None; // Config flags for the Dockspace

    // In this example, we're embedding the Dockspace into an invisible parent window to make it more configurable.
    // We set ImGuiWindowFlags_NoDocking to make sure the parent isn't dockable into because this is handled by the Dockspace.

    // If so, get the main viewport:
    const ImGuiViewport* viewport = ImGui::GetMainViewport();

    // Set the parent window's position, size, and viewport to match that of the main viewport. This is so the parent window
    // completely covers the main viewport, giving it a "full-screen" feel.
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);

    // Set the parent window's styles to match that of the main viewport:
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f); // No corner rounding on the window
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f); // No border around the window


    // If the padding option is disabled, set the parent window's padding size to 0 to effectively hide said padding.
    if (!opt_padding)
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    // Important: note that we proceed even if Begin() returns false (aka window is collapsed).
    // This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
    // all active windows docked into it will lose their parent and become undocked.
    // We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
    // any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
    ImGui::Begin("DockSpace Demo", nullptr);

    // Check if Docking is enabled:
    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
    {
        // If it is, draw the Dockspace with the DockSpace() function.
        // The GetID() function is to give a unique identifier to the Dockspace - here, it's "MyDockSpace".
        ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
    }

    UseImGui::Win1();
    UseImGui::Win2();

}

void UseImGui::Render () {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

}

void UseImGui::Shutdown () {
        // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

}

void UseImGui::Win1(){

    ImGui::Begin("Hello, world!");
    ImGui::Text("This is some useful text.");

    // End the parent window that contains the Dockspace:
    ImGui::End();

}

void UseImGui::Win2(){

    ImGui::Begin("Hello!");
    ImGui::Text("This is some useful text.");

    // End the parent window that contains the Dockspace:
    ImGui::End();

}