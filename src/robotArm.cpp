#include "robotArm.h"

RobotArm::RobotArm(const char* modelPath, Shader shader) : shader(shader) {
    model = LoadModel(modelPath);
    meshVisibility = new bool[model.meshCount];
    meshRotations = new ArmRotation[model.meshCount];
    scale = 0.1f;
    color = RED;
    
    for (int i = 0; i < model.meshCount; i++) {
        meshVisibility[i] = true;
        meshRotations[i] = {0.0f, {0.0f, 1.0f, 0.0f}};
    }
    
    // Ustaw osie obrotu dla poszczególnych części
    meshRotations[0].axis = {0.0f, 1.0f, 0.0f}; // Baza
    meshRotations[1].axis = {0.0f, 0.0f, 1.0f}; // Ramię 1
    meshRotations[2].axis = {0.0f, 0.0f, 1.0f}; // Ramię 2
    meshRotations[3].axis = {0.0f, 0.0f, 1.0f}; // Chwytak

    // Pobierz lokalizację koloru w shaderze
    colorLoc = GetShaderLocation(shader, "materialColor");

    defaultMaterial = LoadMaterialDefault();
    defaultMaterial.shader = shader;
}

RobotArm::~RobotArm() {
    UnloadModel(model);
    delete[] meshVisibility;
    delete[] meshRotations;
    UnloadMaterial(defaultMaterial);
}

Matrix GetHierarchicalTransform(int meshIndex, ArmRotation* rotations) {
    Matrix transform = MatrixIdentity();
    for (int i = 0; i <= meshIndex; i++) {
        transform = MatrixMultiply(transform, 
            MatrixRotate(rotations[i].axis, rotations[i].angle * DEG2RAD));
    }
    return transform;
}

void RobotArm::Draw() {
    BeginShaderMode(shader);
    float colorVec[4] = {color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f};
    SetShaderValue(shader, colorLoc, colorVec, SHADER_UNIFORM_VEC4);
    for (int i = 0; i < model.meshCount; i++) {
        if (meshVisibility[i]) {
            Matrix hierarchicalRotation = GetHierarchicalTransform(i, meshRotations);
            Matrix scaleMatrix = MatrixScale(scale, scale, scale);
            Matrix transform = MatrixMultiply(hierarchicalRotation, scaleMatrix);
            DrawMesh(model.meshes[i], defaultMaterial, transform);
        }
    }
    EndShaderMode();
}

void RobotArm::UpdateRotation(int meshIndex, float angle) {
    if (meshIndex < model.meshCount) {
        meshRotations[meshIndex].angle = angle;
    }
}

void RobotArm::SetMeshVisibility(int meshIndex, bool visible) {
    if (meshIndex < model.meshCount) {
        meshVisibility[meshIndex] = visible;
    }
}

void RobotArm::DrawImGuiControls() {
    if (ImGui::CollapsingHeader("Robot Arm Controls")) {
        if (ImGui::TreeNode("Mesh Visibility")) {
            for (int i = 0; i < model.meshCount; i++) {
                char label[32];
                sprintf(label, "Mesh %d", i);
                ImGui::Checkbox(label, &meshVisibility[i]);
            }
            ImGui::TreePop();
        }
        
        if (ImGui::TreeNode("Joint Rotations")) {
            for (int i = 0; i < model.meshCount; i++) {
                char label[32];
                sprintf(label, "Joint %d", i);
                ImGui::SliderFloat(label, &meshRotations[i].angle, -180.0f, 180.0f);
            }
            ImGui::TreePop();
        }
        
        ImGui::SliderFloat("Scale", &scale, 0.001f, 0.1f);
        ImGui::ColorEdit4("Color", (float*)&color);
    }
}