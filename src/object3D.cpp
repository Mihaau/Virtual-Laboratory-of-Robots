#include "object3D.h"

Object3D::Object3D(const char* modelPath, Shader shader) : 
    shader(shader),
    position({0.0f, 0.0f, 0.0f}),
    rotation({0.0f, 0.0f, 0.0f}),
    scale(1.0f),
    color(WHITE),
    modelPath(modelPath)
{
    model = LoadModel(modelPath);
    
    // Inicjalizacja materiału
    defaultMaterial = LoadMaterialDefault();
    defaultMaterial.shader = shader;
    
    // Konfiguracja shadera
    colorLoc = GetShaderLocation(shader, "materialColor");
    
    // Przypisz materiał do modelu
    for(int i = 0; i < model.materialCount; i++) {
        model.materials[i].shader = shader;
    }

    UpdateTransformMatrix();
}

Object3D::~Object3D() {
    UnloadModel(model);
    UnloadMaterial(defaultMaterial);
}

void Object3D::UpdateTransformMatrix() {
    Matrix translation = MatrixTranslate(position.x, position.y, position.z);
    Matrix rotationX = MatrixRotateX(rotation.x * DEG2RAD);
    Matrix rotationY = MatrixRotateY(rotation.y * DEG2RAD);
    Matrix rotationZ = MatrixRotateZ(rotation.z * DEG2RAD);
    Matrix scaleMatrix = MatrixScale(scale, scale, scale);

    transformMatrix = MatrixIdentity();
    transformMatrix = MatrixMultiply(transformMatrix, translation);
    transformMatrix = MatrixMultiply(transformMatrix, rotationX);
    transformMatrix = MatrixMultiply(transformMatrix, rotationY);
    transformMatrix = MatrixMultiply(transformMatrix, rotationZ);
    transformMatrix = MatrixMultiply(transformMatrix, scaleMatrix);
}

void Object3D::Draw() {
    BeginShaderMode(shader);
    
    // Ustaw kolor materiału
    float colorVec[4] = {
        color.r / 255.0f,
        color.g / 255.0f,
        color.b / 255.0f,
        color.a / 255.0f
    };
    SetShaderValue(shader, colorLoc, colorVec, SHADER_UNIFORM_VEC4);

    // Narysuj model z transformacją
    model.transform = transformMatrix;
    DrawModel(model, Vector3Zero(), 1.0f, color);
    
    EndShaderMode();
}

void Object3D::DrawImGuiControls() {
    if (ImGui::CollapsingHeader(modelPath.c_str())) {
        bool updated = false;
        
        if (ImGui::TreeNode("Transform")) {
            if (ImGui::DragFloat3("Position", (float*)&position, 0.1f)) updated = true;
            if (ImGui::DragFloat3("Rotation", (float*)&rotation, 1.0f, -360.0f, 360.0f)) updated = true;
            if (ImGui::DragFloat("Scale", &scale, 0.001f, 0.001f, 0.1f)) updated = true;
            ImGui::TreePop();
        }
        
        // if (ImGui::ColorEdit4("Color", (float*)&color)) updated = true;
        
        if (updated) {
            UpdateTransformMatrix();
        }
    }
}