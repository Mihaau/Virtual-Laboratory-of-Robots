#include "robotArm.h"

RobotArm::RobotArm(const char *modelPath, Shader shader) : shader(shader)
{
    model = LoadModel(modelPath);
    meshVisibility = new bool[model.meshCount];
    meshRotations = new ArmRotation[model.meshCount];
    scale = 0.01f;
    color = RED;
    showPivotPoints = true;

    for (int i = 0; i < model.meshCount; i++)
    {
        meshVisibility[i] = true;
        meshRotations[i] = {0.0f, {0.0f, 1.0f, 0.0f}};
    }

    // Ustaw osie obrotu dla poszczególnych części
    meshRotations[0].axis = {0.0f, 1.0f, 0.0f}; // Baza
    meshRotations[1].axis = {0.0f, 1.0f, 0.0f}; // Ramię 1
    meshRotations[2].axis = {0.0f, 0.0f, 1.0f}; // Ramię 2
    meshRotations[3].axis = {0.0f, 0.0f, 1.0f}; // Chwytak
    meshRotations[4].axis = {1.0f, 0.0f, 0.0f}; // Obrót chwytaka
    meshRotations[5].axis = {0.0f, 0.0f, 1.0f}; // Obrót chwytaka

    pivotPoints = new Vector3[model.meshCount + 1];

    pivotPoints[0] = {0.0f, 0.0f, 0.0f};   // Baza
    pivotPoints[1] = {0.0f, 100.0f, 0.0f}; // Punkt obrotu ramienia 1
    pivotPoints[2] = {0.0f, 350.0f, 0.0f}; // Punkt obrotu ramienia 2
    pivotPoints[3] = {0.0f, 660.0f, 0.0f}; // Punkt obrotu ramienia 3
    pivotPoints[4] = {0.0f, 660.0f, 0.0f}; // Punkt obrotu ramienia 4
    pivotPoints[5] = {-338.0f, 708.0f, 0.0f}; // Punkt obrotu ramienia 5
    pivotPoints[6] = {-426.0f, 708.0f, 0.0f}; // Punkt obrotu chwytaka

    armLengths = new float[model.meshCount + 1];

    armLengths[0] = Vector3Distance(pivotPoints[0], pivotPoints[1]);
    armLengths[1] = Vector3Distance(pivotPoints[1], pivotPoints[2]);
    armLengths[2] = Vector3Distance(pivotPoints[2], pivotPoints[3]);
    armLengths[3] = Vector3Distance(pivotPoints[3], pivotPoints[4]);
    armLengths[4] = Vector3Distance(pivotPoints[4], pivotPoints[5]);
    armLengths[5] = Vector3Distance(pivotPoints[5], pivotPoints[6]);

    // Pobierz lokalizację koloru w shaderze
    colorLoc = GetShaderLocation(shader, "materialColor");

    defaultMaterial = LoadMaterialDefault();
    defaultMaterial.shader = shader;
}

RobotArm::~RobotArm()
{
    UnloadModel(model);
    delete[] meshRotations;
    delete[] meshVisibility;
    delete[] pivotPoints;
    UnloadMaterial(defaultMaterial);
}

Vector3 TransformAxis(Vector3 axis, Matrix transform)
{
    Vector3 result;
    result.x = axis.x * transform.m0 + axis.y * transform.m4 + axis.z * transform.m8;
    result.y = axis.x * transform.m1 + axis.y * transform.m5 + axis.z * transform.m9;
    result.z = axis.x * transform.m2 + axis.y * transform.m6 + axis.z * transform.m10;
    return Vector3Normalize(result);
}

Matrix GetHierarchicalTransform(int meshIndex, ArmRotation *rotations, Vector3 *pivotPoints)
{
    if (meshIndex < 0)
        return MatrixIdentity();

    // Najpierw pobierz transformację rodzica
    Matrix transform = GetHierarchicalTransform(meshIndex - 1, rotations, pivotPoints);

    // Przesuń do punktu obrotu (w przestrzeni globalnej)
    Vector3 globalPivotPos = Vector3Transform(pivotPoints[meshIndex], transform);
    transform = MatrixMultiply(transform, MatrixTranslate(-globalPivotPos.x, -globalPivotPos.y, -globalPivotPos.z));

    // Oblicz nową oś obrotu na podstawie poprzednich transformacji
    Vector3 newAxis = TransformAxis(rotations[meshIndex].axis, transform);

    // Wykonaj rotację
    transform = MatrixMultiply(transform, MatrixRotate(newAxis, rotations[meshIndex].angle * DEG2RAD));

    // Przesuń z powrotem
    transform = MatrixMultiply(transform, MatrixTranslate(globalPivotPos.x, globalPivotPos.y, globalPivotPos.z));

    return transform;
}


void RobotArm::Draw()
{
    BeginShaderMode(shader);
    float colorVec[4] = {color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f};
    SetShaderValue(shader, colorLoc, colorVec, SHADER_UNIFORM_VEC4);

    for (int i = 0; i < model.meshCount; i++)
    {
        if (meshVisibility[i])
        {
            
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

void RobotArm::DrawPivotPoints()
{
    if (!showPivotPoints)
        return;

    for (int i = 0; i <= model.meshCount; i++)
    {
        // Get hierarchical transform up to parent
        Matrix parentTransform = (i == 0) ? MatrixIdentity() : GetHierarchicalTransform(i - 1, meshRotations, pivotPoints);

        // Transform pivot point by parent transforms first
        Vector3 globalPivotPos = Vector3Transform(pivotPoints[i], parentTransform);

        // Apply scale after all transformations
        globalPivotPos = Vector3Scale(globalPivotPos, scale);

        // Visualize pivot point
        DrawSphere(globalPivotPos, 0.1f, RED);
        DrawSphereWires(globalPivotPos, 0.5f, 8, 8, BLACK);
    }
}

void RobotArm::SetPivotPoint(int index, Vector3 position)
{
    if (index < model.meshCount)
    {
        pivotPoints[index] = position;
    }
}

void RobotArm::UpdateRotation(int meshIndex, float angle)
{
    if (meshIndex < model.meshCount)
    {
        meshRotations[meshIndex].angle = angle;
    }
}

void RobotArm::SetMeshVisibility(int meshIndex, bool visible)
{
    if (meshIndex < model.meshCount)
    {
        meshVisibility[meshIndex] = visible;
    }
}

void RobotArm::DrawImGuiControls()
{
    if (ImGui::CollapsingHeader("Robot Arm Controls"))
    {
        if (ImGui::TreeNode("Mesh Visibility"))
        {
            for (int i = 0; i < model.meshCount; i++)
            {
                char label[32];
                sprintf(label, "Mesh %d", i);
                ImGui::Checkbox(label, &meshVisibility[i]);
            }
            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Joint Rotations"))
        {
            for (int i = 0; i < model.meshCount; i++)
            {
                char label[32];
                sprintf(label, "Joint %d", i);
                ImGui::SliderFloat(label, &meshRotations[i].angle, -180.0f, 180.0f);
            }
            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Pivot Points"))
        {
            ImGui::Checkbox("Show Pivot Points", &showPivotPoints);

            for (int i = 0; i <= model.meshCount; i++)
            {
                char label[50];
                sprintf(label, "Pivot %d", i);
                if (ImGui::TreeNode(label))
                {
                    ImGui::PushItemWidth(200);

                    if (float pos[3] = {pivotPoints[i].x, pivotPoints[i].y, pivotPoints[i].z}; ImGui::DragFloat3("Position", pos, 0.1f))
                    {
                        pivotPoints[i] = {pos[0], pos[1], pos[2]};
                    }

                    ImGui::PopItemWidth();
                    ImGui::TreePop();
                }
            }
            ImGui::TreePop();
        }

                if (ImGui::TreeNode("Inverse Kinematics")) {
            ImGui::Checkbox("Use IK", &useIK);
            
            if (useIK) {
                if (ImGui::DragFloat3("Target Position", (float*)&targetPosition, 0.01f)) {
                    SolveIK();
                }
            }
            
            ImGui::Text("End Effector Position:");
            Vector3 currentPos = CalculateEndEffectorPosition();
            ImGui::Text("X: %.3f Y: %.3f Z: %.3f", currentPos.x, currentPos.y, currentPos.z);
            
            ImGui::TreePop();
        }

        ImGui::SliderFloat("Scale", &scale, 0.001f, 0.1f);
        ImGui::ColorEdit4("Color", (float *)&color);
    }
}

void RobotArm::SolveIK() {
    const float TOLERANCE = 0.001f;
    const int MAX_ITERATIONS = 10;
    
    Vector3 basePos = Vector3Scale(pivotPoints[0], scale);
    Vector3 currentEndEffector = CalculateEndEffectorPosition();
    
    float totalLength = 0.0f;
    for (int i = 0; i <= model.meshCount; i++) {
        totalLength += armLengths[i];
    }
    totalLength *= scale;
    float targetDistance = Vector3Distance(basePos, targetPosition);
    
    if (targetDistance > totalLength) {
        // Cel poza zasięgiem - wyciągnij ramię w kierunku celu
        Vector3 direction = Vector3Normalize(Vector3Subtract(targetPosition, basePos));
        targetPosition = Vector3Add(basePos, Vector3Scale(direction, totalLength));
    }

    for (int iter = 0; iter < MAX_ITERATIONS; iter++) {
        // Forward reaching
        Vector3 joints[7];
        joints[0] = basePos;
        
        for (int i = 1; i < 6; i++) {
            Vector3 dir = Vector3Normalize(Vector3Subtract(joints[i], joints[i-1]));
            float len = Vector3Distance(pivotPoints[i], pivotPoints[i-1]) * scale;
            joints[i] = Vector3Add(joints[i-1], Vector3Scale(dir, len));
        }
        
        // Backward reaching
        joints[6] = targetPosition;
        for (int i = 5; i >= 0; i--) {
            Vector3 dir = Vector3Normalize(Vector3Subtract(joints[i], joints[i+1]));
            float len = Vector3Distance(pivotPoints[i+1], pivotPoints[i]) * scale;
            joints[i] = Vector3Add(joints[i+1], Vector3Scale(dir, len));
        }
        
        // Oblicz kąty dla przegubów
        for (int i = 0; i < model.meshCount; i++) {
            Vector3 v1 = Vector3Subtract(joints[i+1], joints[i]);
            Vector3 v2 = Vector3Subtract(pivotPoints[i+1], pivotPoints[i]);
            
            float angle = atan2(v1.y, v1.x) - atan2(v2.y, v2.x);
            meshRotations[i].angle = angle * RAD2DEG;
            
            // Ogranicz kąty obrotu
            meshRotations[i].angle = ClampAngle(meshRotations[i].angle, -180.0f, 180.0f);
        }
        
        // Sprawdź czy osiągnięto cel
        currentEndEffector = CalculateEndEffectorPosition();
        if (Vector3Distance(currentEndEffector, targetPosition) < TOLERANCE) {
            break;
        }
    }
}

float RobotArm::ClampAngle(float angle, float min, float max) {
    while (angle > max) angle -= 360.0f;
    while (angle < min) angle += 360.0f;
    return fmaxf(min, fminf(max, angle));
}

Vector3 RobotArm::CalculateEndEffectorPosition() {
    Matrix transform = GetHierarchicalTransform(model.meshCount - 1, meshRotations, pivotPoints);
    Vector3 endEffector = Vector3Transform(pivotPoints[model.meshCount], transform);
    return Vector3Scale(endEffector, scale);
}