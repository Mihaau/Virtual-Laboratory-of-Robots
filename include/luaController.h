#include <lua.hpp>
#include "robotArm.h"
#include "logWindow.h"

class LuaController
{
private:
    lua_State *L;
    RobotArm &robotArm;
    LogWindow &logWindow;
    bool isRunning;
    bool stepMode;
    int currentLine;
    float waitTime;                        // Czas oczekiwania dla wait()
    float executeTimer;                    // Timer dla ciągłego wykonywania
    const float EXECUTION_INTERVAL = 0.0f; // Interwał między krokami
    void RegisterFunctions();
    static int lua_setJointRotation(lua_State *L);
    static int lua_wait(lua_State *L);
    static int last_joint;
    static float last_angle;
    static float last_wait;

public:
    LuaController(RobotArm &robot, LogWindow &log);
    ~LuaController();

    void LoadScript(const std::string &code);
    void Step();
    void Run();
    void Stop();
    void SetStepMode(bool enabled);
    int GetCurrentLine() const { return currentLine; }
    bool IsRunning() const { return isRunning; }
    void Update(float deltaTime);
};