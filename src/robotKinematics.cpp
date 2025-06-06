#include "robotKinematics.h"

RobotKinematics::RobotKinematics(Vector3* pivotPoints, float* armLengths, 
                                ArmRotation* meshRotations, int meshCount, float scale)
    : pivotPoints(pivotPoints), armLengths(armLengths), meshRotations(meshRotations),
      meshCount(meshCount), scale(scale), interpolationType(InterpolationType::LINEAR)
{
    isTargetReachable = true;
    lastValidTarget = Vector3Zero();
    targetPosition = Vector3Zero();
}

float RobotKinematics::ClampAngle(float angle, float min, float max) {
    while (angle > max) angle -= 360.0f;
    while (angle < min) angle += 360.0f;
    return fmaxf(min, fminf(max, angle));
}

bool RobotKinematics::IsPositionReachable(const Vector3& position) {
    float totalLength = 0.0f;
    for (int i = 0; i < meshCount; i++) {
        totalLength += armLengths[i];
    }
    totalLength *= scale;

    Vector3 basePos = Vector3Scale(pivotPoints[0], scale);
    float targetDistance = Vector3Distance(basePos, position);

    if (targetDistance > totalLength) {
        return false;
    }

    if (position.y < 0) {
        return false;
    }

    return true;
}

Matrix RobotKinematics::GetHierarchicalTransform(int meshIndex, ArmRotation* rotations, Vector3* pivots) {
    // Przeniesiona implementacja z RobotArm
    if (meshIndex < 0) return MatrixIdentity();
    
    Matrix transform = GetHierarchicalTransform(meshIndex - 1, rotations, pivots);
    Vector3 globalPivotPos = Vector3Transform(pivots[meshIndex], transform);
    transform = MatrixMultiply(transform, MatrixTranslate(-globalPivotPos.x, -globalPivotPos.y, -globalPivotPos.z));
    
    Vector3 newAxis = TransformAxis(rotations[meshIndex].axis, transform);
    transform = MatrixMultiply(transform, MatrixRotate(newAxis, rotations[meshIndex].angle * DEG2RAD));
    transform = MatrixMultiply(transform, MatrixTranslate(globalPivotPos.x, globalPivotPos.y, globalPivotPos.z));
    
    return transform;
}

void RobotKinematics::SolveIK() {
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
    for(int i = 0; i < meshCount; i++) {
        totalLength += armLengths[i] * scale;
    }
    
    float targetDistance = Vector3Distance(basePos, targetPosition);
    if(targetDistance > totalLength) {
        Vector3 direction = Vector3Normalize(Vector3Subtract(targetPosition, basePos));
        targetPosition = Vector3Add(basePos, Vector3Scale(direction, totalLength));
    }

    for(int iter = 0; iter < MAX_ITERATIONS; iter++) {
        Vector3 prevEndEffector = CalculateEndEffectorPosition();
        
        for(int i = 0; i < meshCount - 1; i++) {
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

Vector3 RobotKinematics::TransformAxis(Vector3 axis, Matrix transform) {
    Vector3 result;
    result.x = axis.x * transform.m0 + axis.y * transform.m4 + axis.z * transform.m8;
    result.y = axis.x * transform.m1 + axis.y * transform.m5 + axis.z * transform.m9;
    result.z = axis.x * transform.m2 + axis.y * transform.m6 + axis.z * transform.m10;
    return Vector3Normalize(result);
}

Vector3 RobotKinematics::CalculateEndEffectorPosition() {
    Matrix transform = GetHierarchicalTransform(meshCount - 1, meshRotations, pivotPoints);
    Vector3 endEffector = Vector3Transform(pivotPoints[meshCount], transform);
    return Vector3Scale(endEffector, scale);
}

void RobotKinematics::CalculateTrajectory() {
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
            for(int i = 0; i <= meshCount; i++) {
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