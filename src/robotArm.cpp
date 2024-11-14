#include "robotArm.h"

RobotArm::RobotArm(const char* modelPath, Shader shader) : shader(shader) {
    model = LoadModel(modelPath);
    meshVisibility = new bool[model.meshCount];
    meshRotations = new ArmRotation[model.meshCount];
    scale = 0.1f;
    color = RED;
    showPivotPoints = true;
    
    for (int i = 0; i < model.meshCount; i++) {
        meshVisibility[i] = true;
        meshRotations[i] = {0.0f, {0.0f, 1.0f, 0.0f}};
    }
    
    // Ustaw osie obrotu dla poszczególnych części
    meshRotations[0].axis = {0.0f, 1.0f, 0.0f}; // Baza
    meshRotations[1].axis = {0.0f, 1.0f, 0.0f}; // Ramię 1
    meshRotations[2].axis = {0.0f, 0.0f, 1.0f}; // Ramię 2
    meshRotations[3].axis = {0.0f, 0.0f, 1.0f}; // Chwytak
    meshRotations[4].axis = {1.0f, 0.0f, 0.0f}; // Obrót chwytaka
    meshRotations[5].axis = {0.0f, 1.0f, 0.0f}; // Obrót chwytaka

    pivotPoints = new Vector3[model.meshCount];
    
    pivotPoints[0] = {0.0f, 0.0f, 0.0f};     // Baza
    pivotPoints[1] = {0.0f, 2.0f, 0.0f};     // Punkt obrotu ramienia 1
    pivotPoints[2] = {0.0f, 5.0f, 0.0f};     // Punkt obrotu ramienia 2
    pivotPoints[3] = {0.0f, 8.0f, 0.0f};     // Punkt obrotu chwytaka
    // Pobierz lokalizację koloru w shaderze
    colorLoc = GetShaderLocation(shader, "materialColor");

    defaultMaterial = LoadMaterialDefault();
    defaultMaterial.shader = shader;
}

RobotArm::~RobotArm() {
    UnloadModel(model);
    delete[] meshVisibility;
    delete[] meshRotations;
    delete[] pivotPoints;
    UnloadMaterial(defaultMaterial);
}

Matrix GetHierarchicalTransform(int meshIndex, ArmRotation* rotations, Vector3* pivotPoints) {
    if (meshIndex < 0) return MatrixIdentity();
    
    // Najpierw pobierz transformację rodzica
    Matrix transform = GetHierarchicalTransform(meshIndex - 1, rotations, pivotPoints);
    
    // Przesuń do punktu obrotu (w przestrzeni globalnej)
    transform = MatrixMultiply(transform, 
        MatrixTranslate(pivotPoints[meshIndex].x, pivotPoints[meshIndex].y, pivotPoints[meshIndex].z));
    
    // Wykonaj rotację
    transform = MatrixMultiply(transform,
        MatrixRotate(rotations[meshIndex].axis, rotations[meshIndex].angle * DEG2RAD));
    
    // Przesuń z powrotem
    transform = MatrixMultiply(transform,
        MatrixTranslate(-pivotPoints[meshIndex].x, -pivotPoints[meshIndex].y, -pivotPoints[meshIndex].z));
    
    return transform;
}

void RobotArm::Draw() {
    BeginShaderMode(shader);
    float colorVec[4] = {color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f};
    SetShaderValue(shader, colorLoc, colorVec, SHADER_UNIFORM_VEC4);
    
    for (int i = 0; i < model.meshCount; i++) {
        if (meshVisibility[i]) {
            // Get hierarchical transformation including pivots
            Matrix hierarchicalTransform = GetHierarchicalTransform(i, meshRotations, pivotPoints);
            
            // Apply scale
            Matrix scaleMatrix = MatrixScale(scale, scale, scale);
            Matrix finalTransform = MatrixMultiply(hierarchicalTransform, scaleMatrix);
            
            DrawMesh(model.meshes[i], defaultMaterial, finalTransform);
        }
    }
    EndShaderMode();
}

void RobotArm::DrawPivotPoints(bool show) {
    if (!show) return;
    
    for (int i = 0; i < model.meshCount; i++) {
        // Get hierarchical transform up to parent
        Matrix parentTransform = (i == 0) ? 
            MatrixIdentity() : 
            GetHierarchicalTransform(i - 1, meshRotations, pivotPoints);
            
        // Transform pivot point by parent transforms
        Vector3 globalPivotPos = Vector3Transform(Vector3Scale(pivotPoints[i], scale), parentTransform);
        
        // Invert y position
        globalPivotPos.y = -globalPivotPos.y;
        
        // Visualize pivot point
        DrawSphere(globalPivotPos, 0.1f, YELLOW);
        DrawSphereWires(globalPivotPos, 0.5f, 8, 8, BLACK);
    }
}

void RobotArm::SetPivotPoint(int index, Vector3 position) {
    if (index < model.meshCount) {
        pivotPoints[index] = position;
    }
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

        if (ImGui::TreeNode("Pivot Points")) {
            ImGui::Checkbox("Show Pivot Points", &showPivotPoints);
            
            for (int i = 0; i < model.meshCount; i++) {
                char label[32];
                sprintf(label, "Pivot %d", i);
                if (ImGui::TreeNode(label)) {
                    ImGui::PushItemWidth(100);
                    
                    float pos[3] = {
                        pivotPoints[i].x,
                        pivotPoints[i].y,
                        pivotPoints[i].z
                    };
                    
                    if (ImGui::DragFloat3("Position", pos, 0.1f)) {
                        pivotPoints[i] = {pos[0], pos[1], pos[2]};
                    }
                    
                    ImGui::PopItemWidth();
                    ImGui::TreePop();
                }
            }
            ImGui::TreePop();
        }
        
        ImGui::SliderFloat("Scale", &scale, 0.001f, 0.1f);
        ImGui::ColorEdit4("Color", (float*)&color);
    }
}