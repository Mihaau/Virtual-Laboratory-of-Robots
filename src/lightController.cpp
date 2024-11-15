#include "lightController.h"

LightController::LightController(Shader shader) : 
    lightIntensity(1.0f),
    lightPosition{50.0f, 50.0f, 50.0f},
    shader(shader)
{
    // Inicjalizacja światła kierunkowego
    light = CreateLight(LIGHT_DIRECTIONAL, 
                       lightPosition,
                       Vector3{0.0f, 0.0f, 0.0f}, 
                       WHITE, 
                       shader);

    // Konfiguracja ambient light
    int ambientLoc = GetShaderLocation(shader, "ambient");
    float ambientValues[4] = {0.2f, 0.2f, 0.2f, 1.0f};
    SetShaderValue(shader, ambientLoc, ambientValues, SHADER_UNIFORM_VEC4);
}

void LightController::Update()
{
    light.position = lightPosition;
    light.color = Color{
        (unsigned char)(255.0f * lightIntensity),
        (unsigned char)(255.0f * lightIntensity),
        (unsigned char)(255.0f * lightIntensity),
        255
    };
    UpdateLightValues(shader, light);
}

void LightController::DrawImGuiControls()
{
    if (ImGui::CollapsingHeader("Światło"))
    {
        ImGui::SliderFloat("Intensywność", &lightIntensity, 0.0f, 1.0f);
        ImGui::SliderFloat("Światło X", &lightPosition.x, -100.0f, 100.0f);
        ImGui::SliderFloat("Światło Y", &lightPosition.y, -100.0f, 100.0f);
        ImGui::SliderFloat("Światło Z", &lightPosition.z, -100.0f, 100.0f);
    }
}