#ifndef LIGHT_CONTROLLER_H
#define LIGHT_CONTROLLER_H

#include "raylib.h"
#include "rlights.h"
#include "imgui.h"

class LightController {
public:
    LightController(Shader shader);
    void Update();
    void DrawImGuiControls();
    Light& GetLight() { return light; }

private:
    Light light;
    float lightIntensity;
    Vector3 lightPosition;
    Shader shader;
};

#endif