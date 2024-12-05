#include "luaController.h"

int LuaController::last_joint = 0;
float LuaController::last_angle = 0.0f; 
float LuaController::last_wait = 0.0f;

static RobotArm* g_robotArm = nullptr;

LuaController::LuaController(RobotArm& robot, LogWindow& log) 
    : robotArm(robot), logWindow(log), isRunning(false), stepMode(false), currentLine(0) {
    
    L = luaL_newstate();
    luaL_openlibs(L);
    g_robotArm = &robot; // Zapisz referencję globalnie
    RegisterFunctions();
}

void LuaController::RegisterFunctions() {
    lua_register(L, "setJointRotation", lua_setJointRotation);
    lua_register(L, "wait", lua_wait);
}

int LuaController::lua_setJointRotation(lua_State* L) {
    int joint = lua_tointeger(L, 1);
    float angle = lua_tonumber(L, 2);
    
    if(g_robotArm) {
        g_robotArm->RotateJoint(joint, angle);
        // Zapisz parametry
        last_joint = joint;
        last_angle = angle;
    }
    return 0;
}

int LuaController::lua_wait(lua_State* L) {
    float seconds = lua_tonumber(L, 1);
    last_wait = seconds;
    return lua_yield(L, 0);
}

LuaController::~LuaController() {
    if(L) {
        lua_close(L);
    }
    g_robotArm = nullptr;
}

void LuaController::LoadScript(const std::string& code) {
    Stop();
    if(L) lua_close(L);
    
    L = luaL_newstate();
    luaL_openlibs(L);
    RegisterFunctions();
    
    if(luaL_loadstring(L, code.c_str()) != LUA_OK) {
        std::string error = lua_tostring(L, -1);
        logWindow.AddLog(("Błąd kompilacji: " + error).c_str(), LogLevel::Error);
        lua_pop(L, 1);
        return;
    }
    
    logWindow.AddLog("Skrypt załadowany pomyślnie", LogLevel::Info);
}

void LuaController::Run() {
    if(!isRunning) {
        isRunning = true;
        currentLine = 0;
        logWindow.AddLog("Program uruchomiony", LogLevel::Info);
    }
}

void LuaController::Stop() {
    if(isRunning) {
        isRunning = false;
        logWindow.AddLog("Program zatrzymany", LogLevel::Info);
    }
}

void LuaController::Step() {
    if(!isRunning || !stepMode) {
        logWindow.AddLog("Step(): Program nie jest uruchomiony lub tryb krokowy jest wyłączony", LogLevel::Warning);
        return;
    }

    logWindow.AddLog("Wykonywanie kroku...", LogLevel::Info);
    
    int nresults;
    lua_State* from = nullptr; // Nie używamy coroutines, więc from = nullptr
    int nargs = 0;            // Przy pierwszym wywołaniu nie mamy argumentów
    
    // Poprawne wywołanie dla Lua 5.4
    int status = lua_resume(L, from, nargs, &nresults);
    
    switch(status) {
        case LUA_OK: {
            logWindow.AddLog("Skrypt zakończony", LogLevel::Info);
            Stop();
            break;
        }
        case LUA_YIELD: {
            lua_Debug ar;
            if(lua_getstack(L, 0, &ar)) {
                lua_getinfo(L, "Snl", &ar);
                std::string message = "Wykonano krok: linia " + std::to_string(ar.currentline);
                if(ar.name != nullptr) {
                    if(strcmp(ar.name, "setJointRotation") == 0) {
                        message += " (setJointRotation: przegub=" + std::to_string(last_joint) + 
                                 ", kąt=" + std::to_string(last_angle) + ")";
                    } else if(strcmp(ar.name, "wait") == 0) {
                        message += " (wait: " + std::to_string(last_wait) + "s)";
                    }
                }
                logWindow.AddLog(message.c_str(), LogLevel::Info);
            }
            break;
        }
        default: {
            std::string error = lua_tostring(L, -1);
            logWindow.AddLog(("Błąd wykonania: " + error).c_str(), LogLevel::Error);
            lua_pop(L, 1);
            Stop();
            break;
        }
    }
}

void LuaController::SetStepMode(bool enabled) {
    stepMode = enabled;
    if(enabled) {
        logWindow.AddLog("Włączono tryb krokowy", LogLevel::Info);
    } else {
        logWindow.AddLog("Wyłączono tryb krokowy", LogLevel::Info);
    }
}