#include "object3D.h"

int Object3D::nextId = 0;

Object3D::Object3D(const char *modelPath, Shader shader) : shader(shader),
                                                           position({0.0f, 0.0f, 0.0f}),
                                                           rotation({0.0f, 0.0f, 0.0f}),
                                                           scale(1.0f),
                                                           color(WHITE),
                                                           modelPath(modelPath),
                                                           id(nextId++)
{
    model = LoadModel(modelPath);

    // Inicjalizacja materiału
    defaultMaterial = LoadMaterialDefault();
    defaultMaterial.shader = shader;

    std::string baseName = fs::path(modelPath).stem().string();
    displayName = baseName + " (" + std::to_string(id) + ")";

    // Konfiguracja shadera
    colorLoc = GetShaderLocation(shader, "materialColor");

    // Przypisz materiał do modelu
    for (int i = 0; i < model.materialCount; i++)
    {
        model.materials[i].shader = shader;
    }

    UpdateTransformMatrix();
}

Object3D::~Object3D()
{
    UnloadModel(model);
    UnloadMaterial(defaultMaterial);
}

void Object3D::UpdateTransformMatrix()
{
    Matrix translation = MatrixTranslate(position.x, position.y, position.z);
    Matrix rotationX = MatrixRotateX(rotation.x * DEG2RAD);
    Matrix rotationY = MatrixRotateY(rotation.y * DEG2RAD);
    Matrix rotationZ = MatrixRotateZ(rotation.z * DEG2RAD);
    Matrix scaleMatrix = MatrixScale(scale, scale, scale);

    transformMatrix = MatrixIdentity();
    transformMatrix = MatrixMultiply(transformMatrix, scaleMatrix);
    transformMatrix = MatrixMultiply(transformMatrix, rotationX);
    transformMatrix = MatrixMultiply(transformMatrix, rotationY);
    transformMatrix = MatrixMultiply(transformMatrix, rotationZ);
    transformMatrix = MatrixMultiply(transformMatrix, translation);

}

void Object3D::Draw()
{
    BeginShaderMode(shader);

    // Ustaw kolor materiału
    float colorVec[4] = {
        color.r / 255.0f,
        color.g / 255.0f,
        color.b / 255.0f,
        color.a / 255.0f};
    SetShaderValue(shader, colorLoc, colorVec, SHADER_UNIFORM_VEC4);

    // Narysuj model z transformacją
    model.transform = transformMatrix;
    DrawModel(model, Vector3Zero(), 1.0f, color);

    EndShaderMode();
}

Object3D *Object3D::Create(const char *modelPath, Shader shader)
{
    return new Object3D(modelPath, shader);
}

void Object3D::DrawImGuiControls()
{
    ImGui::PushID(id);
    if (ImGui::CollapsingHeader(displayName.c_str()))
    {
        bool updated = false;

        if (ImGui::TreeNode("Transform"))
        {
            Vector3 uiPosition = position;
            if (ImGui::DragFloat3("Position", (float*)&uiPosition, 0.1f)) {
                // Bezpośrednio przypisujemy pozycję bez przeliczania przez skalę
                position = uiPosition;
                updated = true;
            }
            if (ImGui::DragFloat3("Rotation", (float *)&rotation, 1.0f, -360.0f, 360.0f))
                updated = true;
            if (ImGui::InputFloat("Scale", &scale, 0.01f, 0.1f, "%.3f"))
                updated = true;
            ImGui::TreePop();
        }

        // if (ImGui::ColorEdit4("Color", (float*)&color)) updated = true;

        if (updated)
        {
            UpdateTransformMatrix();
        }
    }
    ImGui::PopID();
}