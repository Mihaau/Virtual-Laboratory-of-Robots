// src/pickRobot.cpp
#include "pickRobot.h"
#include "imgui.h"

PickRobot::PickRobot( ) : selectedRobot(0) {
    ScanRobotsFolder();
}

void PickRobot::ScanRobotsFolder() {
    availableRobots.clear();
    const std::string robotsPath = "assets/robots";

    // Iteruj przez folder z robotami
    for (const auto& entry : fs::directory_iterator(robotsPath)) {
        if (entry.path().extension() == ".glb") {
            RobotModel robot;
            robot.name = entry.path().stem().string();
            robot.modelPath = entry.path().string();
            
            // Sprawdź czy istnieje folder konfiguracyjny
            std::string configFolder = robotsPath + "/." + robot.name;
            std::string configPath = configFolder + "/config.json";
            
            if (fs::exists(configPath)) {
                robot.configPath = configPath;
                availableRobots.push_back(robot);
            }
        }
    }
}

void PickRobot::DrawImGuiControls() {
    for (int i = 0; i < availableRobots.size(); i++) {
        if (ImGui::RadioButton(availableRobots[i].name.c_str(), selectedRobot == i)) {
            selectedRobot = i;
        }
    }
}

std::string PickRobot::GetSelectedRobotPath() const {
    if (selectedRobot >= 0 && selectedRobot < availableRobots.size()) {
        return availableRobots[selectedRobot].modelPath;
    }
    return "";
}

std::string PickRobot::GetSelectedConfigPath() const {
    if (selectedRobot >= 0 && selectedRobot < availableRobots.size()) {
        return availableRobots[selectedRobot].configPath;
    }
    return "";
}