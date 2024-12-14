// include/pickRobot.h
#pragma once
#include <string>
#include <vector>
#include <filesystem>
#include "robotArm.h"
namespace fs = std::filesystem;

class PickRobot {
private:
    struct RobotModel {
        std::string name;
        std::string modelPath;
        std::string configPath;
    };

    std::vector<RobotModel> availableRobots;
    int selectedRobot = -1;
    
public:
    PickRobot();
    void ScanRobotsFolder();
    void DrawImGuiControls();
    std::string GetSelectedRobotPath() const;
    std::string GetSelectedConfigPath() const;
    bool IsRobotSelected() const { return selectedRobot >= 0; }
    int GetSelectedRobotIndex() const { return selectedRobot; }
};