#include "robotArm.h"
#include <fstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

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

Vector3 GetEulerAngles(const Vector3 &ZDirection, const Vector3 &YDirection, const Vector3 &XDirection)
{
    glm::mat3 R = glm::mat3(ToGLM(XDirection), ToGLM(YDirection), ToGLM(ZDirection));
    glm::quat q = glm::quat_cast(R);
    Vector3 angles = FromGLM(glm::degrees(glm::eulerAngles(q)));
    return {angles.z, angles.y, angles.x};
}

Matrix CreateRotationMatrix(Vector3 rotation)
{
    Matrix rotMatrix = MatrixIdentity();
    // Kolejność: Z -> Y -> X (intrinsic rotations)
    rotMatrix = MatrixMultiply(rotMatrix, MatrixRotateZ(rotation.z * DEG2RAD));
    rotMatrix = MatrixMultiply(rotMatrix, MatrixRotateY(rotation.y * DEG2RAD));
    rotMatrix = MatrixMultiply(rotMatrix, MatrixRotateX(rotation.x * DEG2RAD));
    return rotMatrix;
}

Vector3 CalculateStupidAngle(const Vector3 &XDirection, const Vector3 &YDirection, const Vector3 &ZDirection)
{
    // Normalizacja wektorów kierunkowych
    Vector3 x = Vector3Normalize(XDirection);
    Vector3 y = Vector3Normalize(YDirection);
    Vector3 z = Vector3Normalize(ZDirection);

    float yaw = 0.0f;   // obrót wokół osi Y (główna rotacja)
    float pitch = 0.0f; // obrót wokół osi X
    float roll = 0.0f;  // obrót wokół osi Z

    // Obliczenie pitch (obrót wokół X w konwencji YXZ)
    pitch = asin(-z.y);

    // Sprawdzenie możliwego przecięcia kątów
    if (fabs(z.y) < 0.9999f)
    {
        // Standardowe przypadki
        yaw = atan2(x.z, z.z);  // Obrót wokół Y
        roll = atan2(y.x, y.y); // Obrót wokół Z
    }
    else
    {
        // Gimbal lock: ograniczenie kąta
        yaw = atan2(-y.z, y.x);
        roll = 0.0f;
    }

    // Konwersja z radianów na stopnie (jeśli potrzebujesz wyników w stopniach)
    yaw *= RAD2DEG;
    pitch *= RAD2DEG;
    roll *= RAD2DEG;

    return Vector3{yaw, pitch, roll};
}

Vector3 RotateVector(Vector3 vec, Vector3 rotation)
{
    Matrix rotX = MatrixRotateX(rotation.x * DEG2RAD);
    Matrix rotY = MatrixRotateY(rotation.y * DEG2RAD);
    Matrix rotZ = MatrixRotateZ(rotation.z * DEG2RAD);

    Matrix rotationMatrix = MatrixMultiply(MatrixMultiply(rotX, rotY), rotZ);
    return Vector3Transform(vec, rotationMatrix);
}

glm::vec3 rotatePointAroundPivot(
    const glm::vec3 &point,   // Punkt który obracamy
    const glm::vec3 &pivot,   // Punkt wokół którego obracamy
    const glm::vec3 &rotation // Kąty rotacji w stopniach dla osi X, Y, Z
)
{
    // Przesunięcie do początku układu współrzędnych
    glm::vec3 translated = point - pivot;

    // Konwersja kątów ze stopni na radiany
    glm::vec3 rotationRad = glm::radians(rotation);

    // Utworzenie macierzy rotacji dla każdej osi
    glm::mat4 rotationMatrix = glm::mat4(1.0f);
    rotationMatrix = glm::rotate(rotationMatrix, rotationRad.x, glm::vec3(1, 0, 0)); // obrót X
    rotationMatrix = glm::rotate(rotationMatrix, rotationRad.y, glm::vec3(0, 1, 0)); // obrót Y
    rotationMatrix = glm::rotate(rotationMatrix, rotationRad.z, glm::vec3(0, 0, 1)); // obrót Z

    // Aplikowanie rotacji
    glm::vec4 rotated = rotationMatrix * glm::vec4(translated, 1.0f);

    // Przesunięcie z powrotem i zwrócenie wyniku
    return glm::vec3(rotated) + pivot;
}

glm::vec3 axisAngleToEulerXYZ(glm::vec3 axis, float angle)
{
    // Normalize the axis
    axis = glm::normalize(axis);
    float x = axis.x, y = axis.y, z = axis.z;

    // Compute rotation matrix using Rodrigues' formula
    float c = cos(angle);
    float s = sin(angle);
    float t = 1 - c;

    glm::mat3 R = glm::mat3(
        t * x * x + c, t * x * y - s * z, t * x * z + s * y,
        t * x * y + s * z, t * y * y + c, t * y * z - s * x,
        t * x * z - s * y, t * y * z + s * x, t * z * z + c);

    // Extract Euler angles (XYZ convention)
    float theta, phi, psi;
    if (glm::abs(R[2][0] - 1.0f) < glm::epsilon<float>())
    {
        theta = -glm::half_pi<float>();
        phi = 0;
        psi = atan2(-R[0][1], -R[0][2]);
    }
    else if (glm::abs(R[2][0] + 1.0f) < glm::epsilon<float>())
    {
        theta = glm::half_pi<float>();
        phi = 0;
        psi = atan2(R[0][1], R[0][2]);
    }
    else
    {
        theta = asin(-R[2][0]);
        phi = atan2(R[2][1], R[2][2]);
        psi = atan2(R[1][0], R[0][0]);
    }

    return glm::degrees(glm::vec3(phi, theta, psi));
}

RobotArm::RobotArm(const char *modelPath, const char *configPath, Shader shader)
    : shader(shader), logWindow(LogWindow::GetInstance())
{
    // Wczytaj konfigurację z pliku JSON
    std::ifstream configFile(configPath);
    if (!configFile.is_open())
    {
        logWindow.AddLog(
            ("Nie można otworzyć pliku konfiguracji: " + std::string(configPath)).c_str(),
            LogLevel::Error);
        return;
    }

    json config;
    configFile >> config;

    scale = config.value("model", json::object()).value("scale", 1.0f);
    auto joints = config["joints"];
    meshCount = joints.size();

    model = LoadModel(modelPath);
    meshVisibility = new bool[model.meshCount];
    meshRotations = new ArmRotation[model.meshCount];
    pivotPoints = new Vector3[meshCount];
    meshRotations = new ArmRotation[meshCount];
    armLengths = new float[meshCount];

    color = WHITE;
    showPivotPoints = false;
    showTrajectory = false;
    isAnimating = false;

    for (int i = 0; i < model.meshCount + 1; i++)
    {
        meshVisibility[i] = true;
        // meshRotations[i] = {0.0f, {0.0f, 1.0f, 0.0f}};
    }

    for (int i = 0; i < meshCount; ++i)
    {
        pivotPoints[i] = {
            joints[i]["pivot"]["x"].get<float>(),
            joints[i]["pivot"]["y"].get<float>(),
            joints[i]["pivot"]["z"].get<float>()};

        meshRotations[i].axis = {
            joints[i]["axis"]["x"].get<float>(),
            joints[i]["axis"]["y"].get<float>(),
            joints[i]["axis"]["z"].get<float>()};
        meshRotations[i].angle = 0.0f; // Kąt początkowy
    }

    // // Ustaw osie obrotu dla poszczególnych części
    // meshRotations[0].axis = {0.0f, 1.0f, 0.0f}; // Baza
    // meshRotations[1].axis = {0.0f, 1.0f, 0.0f}; // Ramię 1
    // meshRotations[2].axis = {0.0f, 0.0f, 1.0f}; // Ramię 2
    // meshRotations[3].axis = {0.0f, 0.0f, 1.0f}; // Chwytak
    // meshRotations[4].axis = {1.0f, 0.0f, 0.0f}; // Obrót chwytaka
    // meshRotations[5].axis = {0.0f, 0.0f, 1.0f}; // Obrót chwytaka
    // meshRotations[6].axis = {1.0f, 0.0f, 0.0f}; // Obrót chwytaka

    // pivotPoints = new Vector3[model.meshCount + 1];

    // pivotPoints[0] = {0.0f, 0.0f, 0.0f};      // Baza
    // pivotPoints[1] = {0.0f, 100.0f, 0.0f};    // Punkt obrotu ramienia 1
    // pivotPoints[2] = {0.0f, 350.0f, 0.0f};    // Punkt obrotu ramienia 2
    // pivotPoints[3] = {0.0f, 660.0f, 0.0f};    // Punkt obrotu ramienia 3
    // pivotPoints[4] = {-58.0f, 708.0f, 0.0f};  // Punkt obrotu ramienia 4
    // pivotPoints[5] = {-338.0f, 708.0f, 0.0f}; // Punkt obrotu ramienia 5
    // pivotPoints[6] = {-426.0f, 708.0f, 0.0f}; // Punkt obrotu chwytaka

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
    baseDirection = kinematics->GetEndEffectorDirection();
    baseUpDirection = kinematics->GetEndEffectorUpDirection();
    baseSideDirection = kinematics->GetEndEffectorSideDirection();
}

RobotArm::~RobotArm() {
    try {
        // Najpierw zwolnij obiekt jeśli jest trzymany
        if (isGripping && grippedObject) {
            ReleaseObject();
        }
        
        // Wyczyść wskaźniki na obiekty sceny
        sceneObjects = nullptr;
        grippedObject = nullptr;

        // Zwolnij kinematykę
        delete kinematics;
        kinematics = nullptr;

        // Zwolnij dynamicznie alokowane tablice
        delete[] meshVisibility;
        delete[] meshRotations;
        delete[] pivotPoints;
        delete[] armLengths;

        meshVisibility = nullptr;
        meshRotations = nullptr;
        pivotPoints = nullptr;
        armLengths = nullptr;

        // Zwolnij model
        UnloadModel(model);
    }
    catch (const std::exception& e) {
        logWindow.AddLog(("Błąd podczas zwalniania zasobów robota: " + std::string(e.what())).c_str(), LogLevel::Error);
    }
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
    // draw rotatedpoint
    DrawSphere(rotatedPoint, 0.1f, RED);
    if (!tracePath.empty())
    {
        for (size_t i = 0; i < tracePath.size() - 1; i++)
        {
            DrawLine3D(tracePath[i], tracePath[i + 1], PURPLE);
        }
    }
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
        if (ImGui::TreeNode("Path Tracing"))
        {
            if (!isTracing)
            {
                if (ImGui::Button("Start Tracing"))
                {
                    StartTracing();
                }
            }
            else
            {
                if (ImGui::Button("Stop Tracing"))
                {
                    StopTracing();
                }
            }

            if (ImGui::Button("Clear Trace"))
            {
                ClearTrace();
            }

            ImGui::Text("Path points: %d", tracePath.size());
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
            Vector3 upDirection = kinematics->GetEndEffectorUpDirection();
            Vector3 sideDirection = kinematics->GetEndEffectorSideDirection();
            ImGui::Text("Kierunek chwytaka:");
            ImGui::Text("X: %.3f Y: %.3f Z: %.3f",
                        direction.x,
                        direction.y,
                        direction.z);
            ImGui::Text("X: %.3f Y: %.3f Z: %.3f",
                        upDirection.x,
                        upDirection.y,
                        upDirection.z);
            ImGui::Text("X: %.3f Y: %.3f Z: %.3f",
                        sideDirection.x,
                        sideDirection.y,
                        sideDirection.z);
            Vector3 stupidAngle = CalculateStupidAngle(direction, upDirection, sideDirection);
            ImGui::Text("Stupid Angle:");
            ImGui::Text("X: %.3f Y: %.3f Z: %.3f",
                        stupidAngle.x,
                        stupidAngle.y,
                        stupidAngle.z);
            ImGui::Text("rotatedPoint: X: %.3f Y: %.3f Z: %.3f",
                        rotatedPoint.x,
                        rotatedPoint.y,
                        rotatedPoint.z);

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
            // Ustaw końcową pozycję
            kinematics->SetTargetPosition(trajectoryPoints.back());
        }
        else
        {
            // Interpolacja między punktami trajektorii
            int index = (int)(t * (trajectoryPoints.size() - 1));
            index = Clamp(index, 0, trajectoryPoints.size() - 2);
            kinematics->SetTargetPosition(trajectoryPoints[index]);
        }
        kinematics->SolveIK();
    }

    if (isTracing)
    {
        // Dodaj tylko jeśli pozycja się zmieniła
        if (tracePath.empty() || Vector3Distance(tracePath.back(), gripperPosition) > 0.01f)
        {
            tracePath.push_back(gripperPosition);
        }
    }

    if (isGripping && grippedObject)
    {
        Vector3 EndEffectorPos = kinematics->CalculateEndEffectorPosition();
        Vector3 currentXVector = kinematics->GetEndEffectorDirection();
        Vector3 currentYVector = kinematics->GetEndEffectorUpDirection();
        Vector3 currentZVector = kinematics->GetEndEffectorSideDirection();

        // Oblicz kąty Eulera
        Vector3 effectorRotation = CalculateStupidAngle(currentXVector, currentYVector, currentZVector);

        // Zamień kolejność komponentów i dostosuj znaki
        Vector3 finalRotation = {
            effectorRotation.y, // pitch
            effectorRotation.x, // yaw
            -effectorRotation.z // roll
        };

        // Stwórz macierz rotacji dla aktualnej orientacji efektora
        Matrix rotMatrix = CreateRotationMatrix(finalRotation);

        // Przekształć oryginalny offset przez macierz rotacji
        Vector3 rotatedOffset = Vector3Transform(originalGripOffset, rotMatrix);

        // Oblicz finalną pozycję
        Vector3 finalPos = Vector3Add(EndEffectorPos, rotatedOffset);

        // Ustaw rotację i pozycję obiektu
        grippedObject->SetGlobalRotation(finalRotation);
        grippedObject->SetPosition(finalPos);
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

            // Zapisz początkową pozycję względem efektora
            Vector3 EndEffectorPos = kinematics->CalculateEndEffectorPosition();
            Vector3 objPos = grippedObject->GetPosition();

            // Zapisz offset w lokalnym układzie współrzędnych efektora
            originalGripOffset = Vector3Subtract(objPos, EndEffectorPos);

            // Zapisz początkową orientację efektora
            baseDirection = kinematics->GetEndEffectorDirection();
            baseUpDirection = kinematics->GetEndEffectorUpDirection();
            baseSideDirection = kinematics->GetEndEffectorSideDirection();

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
    Vector3 sideDirection = kinematics->GetEndEffectorSideDirection();
    Vector3 scaledUpDirection = Vector3Scale(upDirection, directionLineLength);
    Vector3 scaledDirection = Vector3Scale(direction, directionLineLength);
    Vector3 endPoint = Vector3Add(gripperPosition, scaledDirection);
    Vector3 endPointUp = Vector3Add(gripperPosition, scaledUpDirection);
    Vector3 endPointSide = Vector3Add(gripperPosition, Vector3Scale(sideDirection, directionLineLength));

    // Narysuj linię
    DrawLine3D(gripperPosition, endPoint, BLUE);
    DrawLine3D(gripperPosition, endPointUp, ORANGE);
    DrawLine3D(gripperPosition, endPointSide, GREEN);
}

float RobotArm::GetEndEffectorRoll()
{
    return meshRotations[4].angle;
}

void RobotArm::MoveTo(Vector3 target, InterpolationType type)
{
    kinematics->SetInterpolationType(type);
    kinematics->SetTargetPosition(target);
    kinematics->CalculateTrajectory();

    // Rozpocznij animację
    isAnimating = true;
    animationTime = 0.0f;
}

void RobotArm::StartTracing()
{
    isTracing = true;
    logWindow.AddLog("Rozpoczęto rysowanie ścieżki", LogLevel::Info);
}

void RobotArm::StopTracing()
{
    isTracing = false;
    logWindow.AddLog("Zatrzymano rysowanie ścieżki", LogLevel::Info);
}

void RobotArm::ClearTrace()
{
    tracePath.clear();
    logWindow.AddLog("Wyczyszczono ścieżkę", LogLevel::Info);
}
void RobotArm::Reset()
{
    for (int i = 0; i < meshCount; i++)
    {
        meshRotations[i].angle = 0.0f;  // Resetuj kąty obrotu
    }
    isAnimating = false;
    isTracing = false;
    ClearTrace();
    logWindow.AddLog("Ramię zostało zresetowane", LogLevel::Info);
}