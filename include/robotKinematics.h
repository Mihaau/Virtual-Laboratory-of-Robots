#pragma once
#include "raylib.h"
#include "raymath.h"
#include <vector>

struct ArmRotation
{
    float angle;
    Vector3 axis;
};

enum class InterpolationType
{
    LINEAR,
    PARABOLIC,
    SPLINE
};

class RobotKinematics
{
private:
    Vector3 *pivotPoints;
    float *armLengths;
    ArmRotation *meshRotations;
    int meshCount;
    float scale;
    Vector3 targetPosition;
    bool isTargetReachable;
    Vector3 lastValidTarget;
    std::vector<Vector3> trajectoryPoints;
    std::vector<Vector3> controlPoints;
    InterpolationType interpolationType;

    float ClampAngle(float angle, float min, float max);
    bool IsPositionReachable(const Vector3 &position);

public:
    RobotKinematics(Vector3 *pivotPoints, float *armLengths, ArmRotation *meshRotations,
                    int meshCount, float scale);

    void SolveIK();
    Vector3 CalculateEndEffectorPosition();
    void CalculateTrajectory();
    const std::vector<Vector3> &GetTrajectoryPoints() const { return trajectoryPoints; }
    void SetTargetPosition(const Vector3 &position) { targetPosition = position; }
    Vector3 GetTargetPosition() const { return targetPosition; }
    void SetScale(float newScale) { scale = newScale; }
    void SetInterpolationType(InterpolationType type) { interpolationType = type; }
    const std::vector<Vector3> &GetControlPoints() const { return controlPoints; }
    void SetControlPoints(const std::vector<Vector3> &points) { controlPoints = points; }
    bool IsTargetReachable() const { return isTargetReachable; }
    static Vector3 TransformAxis(Vector3 axis, Matrix transform);
    static Matrix GetHierarchicalTransform(int meshIndex, ArmRotation *rotations, Vector3 *pivots);
    InterpolationType GetInterpolationType() const { return interpolationType; }
    Vector3 GetGripperTransform(const Vector3& objectOffset) const;
    Vector3 GetEndEffectorDirection();
};