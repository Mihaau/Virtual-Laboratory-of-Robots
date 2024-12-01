#include "robotArm.h"

RobotArm::RobotArm(const char *modelPath, Shader shader) : shader(shader)
{
    model = LoadModel(modelPath);
    meshVisibility = new bool[model.meshCount];
    meshRotations = new ArmRotation[model.meshCount];
    scale = 0.01f;
    color = WHITE;
    showPivotPoints = true;
    isTargetReachable = true;
    lastValidTarget = Vector3Zero();
    targetPosition = Vector3Zero();

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
    pivotPoints[4] = {-58.0f, 708.0f, 0.0f}; // Punkt obrotu ramienia 4
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
    static LogWindow& logWindow = LogWindow::GetInstance();
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
    ImGui::Checkbox("Show Trajectory", &showTrajectory);
    
    const char* interpolationTypes[] = { "Linear", "Parabolic", "Spline" };
    int currentType = static_cast<int>(interpolationType);
    if (ImGui::Combo("Interpolation Type", &currentType, interpolationTypes, 3)) {
        interpolationType = static_cast<InterpolationType>(currentType);
        CalculateTrajectory();
    }

    if (ImGui::DragFloat3("Target Position", (float*)&targetPosition, 0.01f)) {
        CalculateTrajectory();
    }

        if (interpolationType == InterpolationType::SPLINE && !controlPoints.empty()) {
        if (ImGui::TreeNode("Control Points")) {
            for (int i = 0; i < controlPoints.size(); i++) {
                char label[32];
                sprintf(label, "Point %d", i);
                if (ImGui::DragFloat3(label, (float*)&controlPoints[i], 0.01f)) {
                    CalculateTrajectory();
                }
                else
                {
                    Vector3 tempTarget = targetPosition;
                    if (ImGui::DragFloat3("Target Position", (float *)&tempTarget, 0.01f))
                    {
                        if (IsPositionReachable(tempTarget))
                        {
                            targetPosition = tempTarget;
                            lastValidTarget = targetPosition;
                            SolveIK();
                        }
                    }
                }

                ImGui::Text("End Effector Position:");
                Vector3 currentPos = CalculateEndEffectorPosition();
                ImGui::Text("X: %.3f Y: %.3f Z: %.3f", currentPos.x, currentPos.y, currentPos.z);
            }
            ImGui::TreePop();
        }
    }
    
    if (ImGui::Button(isAnimating ? "Stop Animation" : "Start Animation")) {
        isAnimating = !isAnimating;
        if (isAnimating) {
            animationTime = 0.0f;
        }
    }
    
    ImGui::Text("End Effector Position:");
    Vector3 currentPos = CalculateEndEffectorPosition();
    ImGui::Text("X: %.3f Y: %.3f Z: %.3f", currentPos.x, currentPos.y, currentPos.z);
    
    ImGui::TreePop();
}

        ImGui::SliderFloat("Scale", &scale, 0.001f, 0.1f);
            float col[4] = {
        color.r/255.0f, 
        color.g/255.0f, 
        color.b/255.0f, 
        color.a/255.0f
    };
    
    if (ImGui::ColorEdit4("Robot Color", col)) {
        color = {
            (unsigned char)(col[0] * 255),
            (unsigned char)(col[1] * 255),
            (unsigned char)(col[2] * 255),
            (unsigned char)(col[3] * 255)
        };
    }
    }
}

bool RobotArm::IsPositionReachable(const Vector3& position) {
    // Oblicz całkowitą długość ramienia
    float totalLength = 0.0f;
    for (int i = 0; i <= model.meshCount; i++) {
        totalLength += armLengths[i];
    }
    totalLength *= scale;

    // Oblicz odległość od bazy do pozycji docelowej
    Vector3 basePos = Vector3Scale(pivotPoints[0], scale);
    float targetDistance = Vector3Distance(basePos, position);

    // Sprawdź czy cel jest w zasięgu ramienia
    if (targetDistance > totalLength) {
        return false;
    }

    // Sprawdź ograniczenia kątowe (uproszczone)
    const float jointLimits[6][2] = {
        {-180.0f, 180.0f}, // Baza
        {-90.0f, 90.0f},   // Ramię 1
        {-120.0f, 120.0f}, // Ramię 2
        {-120.0f, 120.0f}, // Ramię 3
        {-180.0f, 180.0f}, // Obrót chwytaka
        {-90.0f, 90.0f}    // Chwytak
    };

    // Sprawdź czy pozycja jest powyżej podłoża
    if (position.y < 0) {
        return false;
    }

    return true;
}

void RobotArm::SolveIK() {
    const float TOLERANCE = 0.001f;
    const int MAX_ITERATIONS = 10;
    const float DAMPING = 0.1f; // Współczynnik tłumienia dla stabilności
    
    Vector3 basePos = Vector3Scale(pivotPoints[0], scale);
    Vector3 currentEndEffector = CalculateEndEffectorPosition();
    
    // Ograniczenia kątowe dla każdego przegubu (min, max)
    const float jointLimits[6][2] = {
        {-180.0f, 180.0f}, // Baza
        {-90.0f, 90.0f},   // Ramię 1
        {-120.0f, 120.0f}, // Ramię 2
        {-120.0f, 120.0f}, // Ramię 3
        {-180.0f, 180.0f}, // Ramię 4
        {-90.0f, 90.0f}    // Chwytak
    };

    // Sprawdź osiągalność celu
    float totalLength = 0.0f;
    for(int i = 0; i < model.meshCount; i++) {
        totalLength += armLengths[i] * scale;
    }
    
    float targetDistance = Vector3Distance(basePos, targetPosition);
    if(targetDistance > totalLength) {
        Vector3 direction = Vector3Normalize(Vector3Subtract(targetPosition, basePos));
        targetPosition = Vector3Add(basePos, Vector3Scale(direction, totalLength));
    }

    for(int iter = 0; iter < MAX_ITERATIONS; iter++) {
        Vector3 prevEndEffector = CalculateEndEffectorPosition();
        
        for(int i = 0; i < model.meshCount - 1; i++) {
            Matrix currentTransform = GetHierarchicalTransform(i, meshRotations, pivotPoints);
            Vector3 jointPos = Vector3Transform(pivotPoints[i], currentTransform);
            jointPos = Vector3Scale(jointPos, scale);
            
            Vector3 currentEndEffector = CalculateEndEffectorPosition();
            
            // Wektory do obliczeń
            Vector3 toEndEffector = Vector3Normalize(Vector3Subtract(currentEndEffector, jointPos));
            Vector3 toTarget = Vector3Normalize(Vector3Subtract(targetPosition, jointPos));
            
            // Oś obrotu w przestrzeni przegubu
            Vector3 jointAxis = meshRotations[i].axis;
            Vector3 globalAxis = TransformAxis(jointAxis, currentTransform);
            
            // Oblicz kąt obrotu
            float dotProduct = Vector3DotProduct(toEndEffector, toTarget);
            dotProduct = Clamp(dotProduct, -1.0f, 1.0f);
            float rotationAngle = acosf(dotProduct) * RAD2DEG;
            
            // Określ kierunek obrotu
            Vector3 cross = Vector3CrossProduct(toEndEffector, toTarget);
            float direction = Vector3DotProduct(cross, globalAxis);
            
            // Zastosuj tłumienie do kąta
            rotationAngle *= DAMPING;
            
            if(fabs(direction) > 0.001f) {
                float newAngle = meshRotations[i].angle;
                if(direction > 0) {
                    newAngle += rotationAngle;
                } else {
                    newAngle -= rotationAngle;
                }
                
                // Ogranicz kąt do dozwolonego zakresu
                newAngle = ClampAngle(newAngle, jointLimits[i][0], jointLimits[i][1]);
                meshRotations[i].angle = newAngle;
            }
        }
        
        // Sprawdź postęp
        Vector3 newEndEffector = CalculateEndEffectorPosition();
        float improvement = Vector3Distance(prevEndEffector, targetPosition) - 
                          Vector3Distance(newEndEffector, targetPosition);
        
        if(improvement < TOLERANCE || 
           Vector3Distance(newEndEffector, targetPosition) < TOLERANCE) {
            break;
        }
    }
}

float RobotArm::ClampAngle(float angle, float min, float max)
{
    while (angle > max)
        angle -= 360.0f;
    while (angle < min)
        angle += 360.0f;
    return fmaxf(min, fminf(max, angle));
}

Vector3 RobotArm::CalculateEndEffectorPosition()
{
    Matrix transform = GetHierarchicalTransform(model.meshCount - 1, meshRotations, pivotPoints);
    Vector3 endEffector = Vector3Transform(pivotPoints[model.meshCount], transform);
    return Vector3Scale(endEffector, scale);
}

void RobotArm::CalculateTrajectory() {
    trajectoryPoints.clear();
    
    Vector3 startPos = CalculateEndEffectorPosition();
    Vector3 endPos = targetPosition;
    float pathLength = Vector3Distance(startPos, endPos);
    const int numPoints = 50;

    switch(interpolationType) {
        case InterpolationType::LINEAR:
            // Interpolacja liniowa
            for(int i = 0; i <= numPoints; i++) {
                float t = i / (float)numPoints;
                Vector3 point = {
                    startPos.x + (endPos.x - startPos.x) * t,
                    startPos.y + (endPos.y - startPos.y) * t,
                    startPos.z + (endPos.z - startPos.z) * t
                };
                trajectoryPoints.push_back(point);
            }
            break;

        case InterpolationType::PARABOLIC: {
            // Istniejąca implementacja paraboliczna
            float pathLength = Vector3Distance(startPos, endPos);
            float heightOffset = pathLength * 0.5f;
            float maxHeight = 0;
            for(int i = 0; i <= model.meshCount; i++) {
                Vector3 point = Vector3Transform(pivotPoints[i], 
                    GetHierarchicalTransform(i-1, meshRotations, pivotPoints));
                maxHeight = fmaxf(maxHeight, point.y);
            }
            
            Vector3 midPoint = {
                (startPos.x + endPos.x) * 0.5f,
                fmaxf(maxHeight + heightOffset, ((startPos.y + endPos.y) * 0.5f + heightOffset)),
                (startPos.z + endPos.z) * 0.5f
            };
            
            for(int i = 0; i <= numPoints; i++) {
                float t = i / (float)numPoints;
                Vector3 point = {
                    (1-t)*(1-t)*startPos.x + 2*(1-t)*t*midPoint.x + t*t*endPos.x,
                    (1-t)*(1-t)*startPos.y + 2*(1-t)*t*midPoint.y + t*t*endPos.y,
                    (1-t)*(1-t)*startPos.z + 2*(1-t)*t*midPoint.z + t*t*endPos.z
                };
                trajectoryPoints.push_back(point);
            }
            break;
        }

        case InterpolationType::SPLINE: {
            // Cubic spline interpolation
            if(controlPoints.empty()) {
                // Poprawiona inicjalizacja punktów kontrolnych
                controlPoints.clear();
                controlPoints.push_back(startPos);
                controlPoints.push_back(Vector3{
                    (startPos.x + endPos.x)*0.25f,
                    startPos.y + pathLength*0.3f,
                    (startPos.z + endPos.z)*0.25f
                });
                controlPoints.push_back(Vector3{
                    (startPos.x + endPos.x)*0.75f,
                    startPos.y + pathLength*0.3f,
                    (startPos.z + endPos.z)*0.75f
                });
                controlPoints.push_back(endPos);
            }
            
            for(int i = 0; i <= numPoints; i++) {
                float t = i / (float)numPoints;
                // Cubic Bezier
                float u = 1.0f - t;
                Vector3 point = {
                    u*u*u * controlPoints[0].x + 
                    3*u*u*t * controlPoints[1].x + 
                    3*u*t*t * controlPoints[2].x + 
                    t*t*t * controlPoints[3].x,
                    
                    u*u*u * controlPoints[0].y + 
                    3*u*u*t * controlPoints[1].y + 
                    3*u*t*t * controlPoints[2].y + 
                    t*t*t * controlPoints[3].y,
                    
                    u*u*u * controlPoints[0].z + 
                    3*u*u*t * controlPoints[1].z + 
                    3*u*t*t * controlPoints[2].z + 
                    t*t*t * controlPoints[3].z
                };
                trajectoryPoints.push_back(point);
            }
            break;
        }
    }
}

void RobotArm::DrawTrajectory() {
    if (!showTrajectory || trajectoryPoints.empty()) return;

    // Draw line segments between points
    for(size_t i = 0; i < trajectoryPoints.size() - 1; i++) {
        DrawLine3D(trajectoryPoints[i], trajectoryPoints[i + 1], YELLOW);
    }
    
    // Draw spheres at start and end points
    DrawSphere(trajectoryPoints.front(), 0.1f, BLUE);
    DrawSphere(trajectoryPoints.back(), 0.1f, RED);
}

void RobotArm::Update() {
    if (isAnimating && !trajectoryPoints.empty()) {
        // Aktualizuj czas animacji (GetFrameTime() daje czas między klatkami)
        animationTime += GetFrameTime();
        
        // Normalizuj czas do zakresu 0-1
        float t = animationTime / ANIMATION_DURATION;
        
        // Zakończ animację gdy osiągnie koniec
        if (t >= 1.0f) {
            isAnimating = false;
            t = 1.0f;
        }
        
        // Znajdź indeks punktu w trajektorii
        int index = (int)(t * (trajectoryPoints.size() - 1));
        index = Clamp(index, 0, trajectoryPoints.size() - 2);
        
        // Ustaw cel IK na aktualny punkt trajektorii
        targetPosition = trajectoryPoints[index];
        
        // Rozwiąż IK dla nowej pozycji
        SolveIK();
    }
}