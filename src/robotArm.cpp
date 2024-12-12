#include "robotArm.h"

glm::vec3 ToGLM(const Vector3 &v)
{
    return glm::vec3(v.x, v.y, v.z);
}

Vector3 FromGLM(const glm::vec3 &v)
{
    return Vector3{v.x, v.y, v.z};
}

// Funkcja dla konwersji rotacji (zakładając że Vector3 przechowuje kąty Eulera)
glm::quat ToGLMQuat(const Vector3 &euler)
{
    return glm::quat(glm::vec3(euler.x, euler.y, euler.z));
}

Vector3 FromGLMQuat(const glm::quat &q)
{
    return Vector3{glm::eulerAngles(q).x, glm::eulerAngles(q).y, glm::eulerAngles(q).z};
}

Vector3 GetEulerAngles(const Vector3& direction, const Vector3& upDirection)
{
    // Konwersja wektorów wejściowych na glm::vec3
    glm::vec3 forward = glm::normalize(ToGLM(direction));
    glm::vec3 right = glm::normalize(glm::cross(ToGLM(upDirection), forward));
    glm::vec3 up = glm::cross(forward, right);

    // Utworzenie macierzy rotacji z wektorów bazowych
    glm::mat3 rotMatrix;
    rotMatrix[0] = right;   // pierwsza kolumna
    rotMatrix[1] = up;      // druga kolumna
    rotMatrix[2] = forward; // trzecia kolumna

    // Konwersja macierzy na quaternion
    glm::quat rotation = glm::quat_cast(rotMatrix);
    
    // Konwersja quaterniona na kąty Eulera (w radianach)
    glm::vec3 eulerAngles = glm::degrees(glm::eulerAngles(rotation));
    
    // Normalizacja kątów do zakresu [-180, 180]
    Vector3 angles = FromGLM(eulerAngles);
    angles.x = fmod(angles.x + 180.0f, 360.0f) - 180.0f;
    angles.y = fmod(angles.y + 180.0f, 360.0f) - 180.0f;
    angles.z = fmod(angles.z + 180.0f, 360.0f) - 180.0f;

    return angles;
}


RobotArm::RobotArm(const char *modelPath, Shader shader)
    : shader(shader), logWindow(LogWindow::GetInstance())
{
    model = LoadModel(modelPath);
    meshVisibility = new bool[model.meshCount];
    meshRotations = new ArmRotation[model.meshCount];
    scale = 0.01f;
    color = WHITE;
    showPivotPoints = false;
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

    gripperRadius = 20.0f;
    isColliding = false;
    gripperColor = GREEN;
}

RobotArm::~RobotArm()
{
    UnloadModel(model);
    delete[] meshRotations;
    delete[] meshVisibility;
    delete[] pivotPoints;
    UnloadMaterial(defaultMaterial);
    delete kinematics;
    if (L)
        lua_close(L);
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
            Matrix hierarchicalTransform = RobotKinematics::GetHierarchicalTransform(i, meshRotations, pivotPoints);
            Matrix scaleMatrix = MatrixScale(scale, scale, scale);
            Matrix finalTransform = MatrixMultiply(hierarchicalTransform, scaleMatrix);
            DrawMesh(model.meshes[i], defaultMaterial, finalTransform);
        }
    }
    EndShaderMode();
    DrawTrajectory();
    DrawGripper();
    DrawGripperDirection();
}

void RobotArm::DrawPivotPoints()
{
    if (!showPivotPoints)
        return;

    for (int i = 0; i <= model.meshCount; i++)
    {
        Matrix parentTransform = (i == 0) ? MatrixIdentity() : RobotKinematics::GetHierarchicalTransform(i - 1, meshRotations, pivotPoints);
        Vector3 globalPivotPos = Vector3Transform(pivotPoints[i], parentTransform);
        globalPivotPos = Vector3Scale(globalPivotPos, scale);

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
        if (ImGui::TreeNode("Chwytak"))
        {
            // Wyświetl aktualny stan
            ImGui::Text("Stan: %s", isColliding ? "Wykryto obiekt" : "Brak kolizji");
            ImGui::Text("Chwytanie: %s", isGripping ? "Aktywne" : "Nieaktywne");
            ImGui::Text("Offset chwytaka: X:%.3f Y:%.3f Z:%.3f",
                        gripOffset.x, gripOffset.y, gripOffset.z);

            // Przyciski sterowania chwytakiem
            if (isColliding && !isGripping)
            {
                if (ImGui::Button("Chwyć obiekt"))
                {
                    GripObject();
                    logWindow.AddLog("Chwycono obiekt", LogLevel::Info);
                }
            }
            else if (isGripping)
            {
                if (ImGui::Button("Puść obiekt"))
                {
                    ReleaseObject();
                    logWindow.AddLog("Puszczono obiekt", LogLevel::Info);
                }
            }

            // Wyświetl informacje o pozycji chwytaka
            ImGui::Text("Pozycja chwytaka:");
            ImGui::Text("X: %.2f Y: %.2f Z: %.2f",
                        gripperPosition.x,
                        gripperPosition.y,
                        gripperPosition.z);

            ImGui::TreePop();
        }
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

            Vector3 direction = kinematics->GetEndEffectorDirection();
            ImGui::Text("Kierunek chwytaka:");
            ImGui::Text("X: %.3f Y: %.3f Z: %.3f",
                        direction.x,
                        direction.y,
                        direction.z);
            Vector3 upDirection = kinematics->GetEndEffectorUpDirection();
            Vector3 euler = GetEulerAngles(direction, upDirection);
            ImGui::Text("Kąty Eulera:");
            ImGui::Text("X: %.3f Y: %.3f Z: %.3f",
                        euler.x,
                        euler.y,
                        euler.z);


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
                        sprintf(label, "Point %zd", i);
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
        animationTime += GetFrameTime();

        float t = animationTime / ANIMATION_DURATION;

        if (t >= 1.0f)
        {
            isAnimating = false;
            t = 1.0f;
        }

        int index = (int)(t * (trajectoryPoints.size() - 1));
        index = Clamp(index, 0, trajectoryPoints.size() - 2);

        kinematics->SetTargetPosition(trajectoryPoints[index]);
        kinematics->SolveIK();
    }

    if (isGripping && grippedObject)
    {
        // Pobierz aktualne wektory kierunku
        Vector3 currentDirection = kinematics->GetEndEffectorDirection();
        Vector3 currentUpDirection = kinematics->GetEndEffectorUpDirection();
        
        // Oblicz rotację bazową
        Vector3 baseRotation = GetEulerAngles(baseDirection, baseUpDirection);
        
        // Oblicz aktualną rotację
        Vector3 currentRotation = GetEulerAngles(currentDirection, currentUpDirection);
        
        // Oblicz różnicę rotacji
        Vector3 rotationDiff;
        rotationDiff.x = currentRotation.x - baseRotation.x;
        rotationDiff.y = currentRotation.y - baseRotation.y;
        rotationDiff.z = currentRotation.z - baseRotation.z;
        
        // Ustaw pozycję i rotację obiektu
        grippedObject->SetRotation({rotationDiff.y, rotationDiff.x, rotationDiff.z});
        grippedObject->SetPosition(gripperPosition);
    }
}

void RobotArm::SetScale(float newScale)
{
    scale = newScale;
    kinematics->SetScale(newScale);
}

void RobotArm::RotateJoint(int jointIndex, float angle)
{
    if (jointIndex >= 0 && jointIndex < model.meshCount)
    {
        UpdateRotation(jointIndex, angle);
    }
}

void RobotArm::CheckCollisions(const std::vector<Object3D *> &objects)
{
    static LogWindow &logWindow = LogWindow::GetInstance();
    static bool wasColliding = false; // Do śledzenia poprzedniego stanu kolizji

    Matrix parentTransform = RobotKinematics::GetHierarchicalTransform(model.meshCount - 1, meshRotations, pivotPoints);
    gripperPosition = Vector3Transform(pivotPoints[model.meshCount], parentTransform);
    gripperPosition = Vector3Scale(gripperPosition, scale);

    bool currentlyColliding = false;
    std::string collidingObjectName;

    for (const auto *obj : objects)
    {
        const Model &objModel = obj->GetModel();

        BoundingBox objBox = GetMeshBoundingBox(objModel.meshes[0]);
        Vector3 objPos = obj->GetPosition();
        float objScale = obj->GetScale();

        Vector3 min = Vector3Add(Vector3Scale(objBox.min, objScale), objPos);
        Vector3 max = Vector3Add(Vector3Scale(objBox.max, objScale), objPos);
        BoundingBox transformedBox = {min, max};

        if (CheckCollisionBoxSphere(transformedBox, gripperPosition, gripperRadius * scale))
        {
            if (isGripping)
            {
                return;
            }
            for (int i = 0; i < objModel.meshCount; i++)
            {
                Ray ray;
                ray.position = gripperPosition;
                ray.direction = {0, 0, 0};

                const int numRays = 8;
                for (int j = 0; j < numRays; j++)
                {
                    float angle = (2.0f * PI * j) / numRays;
                    ray.direction = {cosf(angle), 0.0f, sinf(angle)};

                    Matrix transform = obj->GetTransform();
                    RayCollision collision = GetRayCollisionMesh(ray, objModel.meshes[i], transform);
                    if (collision.hit && collision.distance < gripperRadius * scale)
                    {
                        currentlyColliding = true;
                        collidingObjectName = obj->GetModelPath();
                        break;
                    }
                }
                if (currentlyColliding)
                    break;
            }
        }
        if (currentlyColliding)
            break;
    }

    // Logowanie zmian stanu kolizji
    if (currentlyColliding && !wasColliding)
    {
        logWindow.AddLog(("Wykryto kolizję z obiektem: " + collidingObjectName).c_str(), LogLevel::Warning);
    }
    else if (!currentlyColliding && wasColliding)
    {
        logWindow.AddLog("Koniec kolizji", LogLevel::Info);
    }

    isColliding = currentlyColliding;
    wasColliding = currentlyColliding;
    gripperColor = isColliding ? RED : GREEN;
}

void RobotArm::DrawGripper()
{
    // Dostosuj promień do skali modelu
    float scaledRadius = gripperRadius * scale;
    DrawSphere(gripperPosition, scaledRadius, gripperColor);
}

void RobotArm::GripObject()
{
    if (!isColliding || isGripping || !sceneObjects)
        return;

    for (const auto *obj : *sceneObjects)
    {
        const Model &objModel = obj->GetModel();
        BoundingBox objBox = GetMeshBoundingBox(objModel.meshes[0]);
        Vector3 objPos = obj->GetPosition();
        float objScale = obj->GetScale();

        BoundingBox transformedBox = {
            Vector3Add(Vector3Scale(objBox.min, objScale), objPos),
            Vector3Add(Vector3Scale(objBox.max, objScale), objPos)};

        if (CheckCollisionBoxSphere(transformedBox, gripperPosition, gripperRadius * scale))
        {
            grippedObject = const_cast<Object3D *>(obj);
            isGripping = true;

            gripOffset = Vector3Subtract(objPos, gripperPosition);
            baseDirection = kinematics->GetEndEffectorDirection();
            baseUpDirection = kinematics->GetEndEffectorUpDirection();

            logWindow.AddLog("Obiekt chwycony", LogLevel::Info);
            break;
        }
    }
}
void RobotArm::ReleaseObject()
{
    if (!isGripping)
        return;
    grippedObject = nullptr;
    isGripping = false;
    logWindow.AddLog("Obiekt puszczony", LogLevel::Info);
}

void RobotArm::DrawGripperDirection()
{
    // Długość linii wskazującej kierunek
    const float directionLineLength = gripperRadius * 4.0f * scale;

    // Oblicz punkt końcowy linii
    Vector3 direction = kinematics->GetEndEffectorDirection();
    Vector3 upDirection = kinematics->GetEndEffectorUpDirection();
    Vector3 scaledUpDirection = Vector3Scale(upDirection, directionLineLength);
    Vector3 scaledDirection = Vector3Scale(direction, directionLineLength);
    Vector3 endPoint = Vector3Add(gripperPosition, scaledDirection);
    Vector3 endPointUp = Vector3Add(gripperPosition, scaledUpDirection);

    // Narysuj linię
    DrawLine3D(gripperPosition, endPoint, BLUE);
    DrawLine3D(gripperPosition, endPointUp, ORANGE);
}

float RobotArm::GetEndEffectorRoll()
{
    return meshRotations[4].angle;
}