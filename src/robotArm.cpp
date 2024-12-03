#include "robotArm.h"

RobotArm::RobotArm(const char *modelPath, Shader shader) : shader(shader)
{
    model = LoadModel(modelPath);
    meshVisibility = new bool[model.meshCount];
    meshRotations = new ArmRotation[model.meshCount];
    scale = 0.01f;
    color = WHITE;
    showPivotPoints = true;
    showTrajectory = false;
    isAnimating = false;

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

    pivotPoints[0] = {0.0f, 0.0f, 0.0f};      // Baza
    pivotPoints[1] = {0.0f, 100.0f, 0.0f};    // Punkt obrotu ramienia 1
    pivotPoints[2] = {0.0f, 350.0f, 0.0f};    // Punkt obrotu ramienia 2
    pivotPoints[3] = {0.0f, 660.0f, 0.0f};    // Punkt obrotu ramienia 3
    pivotPoints[4] = {-58.0f, 708.0f, 0.0f};  // Punkt obrotu ramienia 4
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
    kinematics = new RobotKinematics(pivotPoints, armLengths, meshRotations, model.meshCount, scale);
}

RobotArm::~RobotArm()
{
    UnloadModel(model);
    delete[] meshRotations;
    delete[] meshVisibility;
    delete[] pivotPoints;
    UnloadMaterial(defaultMaterial);
    delete kinematics;
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
    DrawTrajectory();
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
    static LogWindow &logWindow = LogWindow::GetInstance();
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

        if (ImGui::TreeNode("Inverse Kinematics"))
        {
            ImGui::Checkbox("Show Trajectory", &showTrajectory);

            const char *interpolationTypes[] = {"Linear", "Parabolic", "Spline"};
            int currentType = static_cast<int>(kinematics->GetInterpolationType());
            if (ImGui::Combo("Interpolation Type", &currentType, interpolationTypes, 3))
            {
                kinematics->SetInterpolationType(static_cast<InterpolationType>(currentType));
                kinematics->CalculateTrajectory();
            }

            Vector3 targetPos = kinematics->GetTargetPosition();
            if (ImGui::DragFloat3("Target Position", (float *)&targetPos, 0.01f))
            {
                kinematics->SetTargetPosition(targetPos);
                kinematics->CalculateTrajectory();
            }

            if (kinematics->GetInterpolationType() == InterpolationType::SPLINE)
            {
                if (ImGui::TreeNode("Control Points"))
                {
                    auto controlPoints = kinematics->GetControlPoints();
                    bool updated = false;

                    for (size_t i = 0; i < controlPoints.size(); i++)
                    {
                        char label[32];
                        sprintf(label, "Point %d", i);
                        Vector3 point = controlPoints[i];
                        if (ImGui::DragFloat3(label, (float *)&point, 0.01f))
                        {
                            controlPoints[i] = point;
                            updated = true;
                        }
                    }

                    if (updated)
                    {
                        kinematics->SetControlPoints(controlPoints);
                        kinematics->CalculateTrajectory();
                    }
                    ImGui::TreePop();
                }
            }

            if (ImGui::Button(isAnimating ? "Stop Animation" : "Start Animation"))
            {
                isAnimating = !isAnimating;
                if (isAnimating)
                {
                    animationTime = 0.0f;
                }
            }

            ImGui::Text("End Effector Position:");
            Vector3 currentPos = kinematics->CalculateEndEffectorPosition();
            ImGui::Text("X: %.3f Y: %.3f Z: %.3f", currentPos.x, currentPos.y, currentPos.z);

            ImGui::TreePop();
        }

        if (ImGui::SliderFloat("Scale", &scale, 0.001f, 0.1f))
        {
            kinematics->SetScale(scale);
            kinematics->CalculateTrajectory(); // Przelicz trajektorię dla nowej skali
        }
        float col[4] = {
            color.r / 255.0f,
            color.g / 255.0f,
            color.b / 255.0f,
            color.a / 255.0f};

        if (ImGui::ColorEdit4("Robot Color", col))
        {
            color = {
                (unsigned char)(col[0] * 255),
                (unsigned char)(col[1] * 255),
                (unsigned char)(col[2] * 255),
                (unsigned char)(col[3] * 255)};
        }
    }
}
void RobotArm::DrawTrajectory()
{
    if (!showTrajectory || kinematics->GetTrajectoryPoints().empty())
        return;

    const auto &points = kinematics->GetTrajectoryPoints();
    for (size_t i = 0; i < points.size() - 1; i++)
    {
        DrawLine3D(points[i], points[i + 1], YELLOW);
    }
    DrawSphere(points.front(), 0.1f, BLUE);
    DrawSphere(points.back(), 0.1f, RED);
}

void RobotArm::Update()
{
    const auto &trajectoryPoints = kinematics->GetTrajectoryPoints();

    if (isAnimating && !trajectoryPoints.empty())
    {
        // Aktualizuj czas animacji (GetFrameTime() daje czas między klatkami)
        animationTime += GetFrameTime();

        // Normalizuj czas do zakresu 0-1
        float t = animationTime / ANIMATION_DURATION;

        // Zakończ animację gdy osiągnie koniec
        if (t >= 1.0f)
        {
            isAnimating = false;
            t = 1.0f;
        }

        // Znajdź indeks punktu w trajektorii
        int index = (int)(t * (trajectoryPoints.size() - 1));
        index = Clamp(index, 0, trajectoryPoints.size() - 2);

        // Ustaw cel IK na aktualny punkt trajektorii
        kinematics->SetTargetPosition(trajectoryPoints[index]);

        // Rozwiąż IK dla nowej pozycji
        kinematics->SolveIK();
    }
}

void RobotArm::SetScale(float newScale)
{
    scale = newScale;
    kinematics->SetScale(newScale);
}