#include <lua.hpp>
#include "robotArm.h"
#include "logWindow.h"

class LuaController
{
private:
    lua_State *L;
    RobotArm& robotArm;
    LogWindow &logWindow;
    static RobotArm* g_robotArm;
    bool isRunning;
    bool stepMode;
    int currentLine;
    float waitTime;                        // Czas oczekiwania dla wait()
    float executeTimer;                    // Timer dla ciągłego wykonywania
    const float EXECUTION_INTERVAL = 0.0f; // Interwał między krokami
    void RegisterFunctions();
    static int lua_setJointRotation(lua_State *L);
    static int lua_wait(lua_State *L);
    static int lua_moveLinear(lua_State *L);
    static int lua_moveParabolic(lua_State *L);
    static int lua_moveSpline(lua_State *L);
    static int lua_gripObject(lua_State *L);
    static int lua_releaseObject(lua_State *L);
    static int last_joint;
    static float last_angle;
    static float last_wait;
        void ResetLuaState() {
    if (L && L != mainThread) {
        // Zamknij tylko podrzędny wątek, jeśli istnieje
        lua_close(L);
    }
    L = lua_newthread(mainThread); // Stwórz nowy wątek wykonawczy
    }
        lua_State* mainThread;  // Główny wątek Lua
    void InitLuaState(); 

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