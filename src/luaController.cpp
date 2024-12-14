#include "luaController.h"

int LuaController::last_joint = 0;
float LuaController::last_angle = 0.0f; 
float LuaController::last_wait = 0.0f;

RobotArm* LuaController::g_robotArm = nullptr;

LuaController::LuaController(RobotArm& robot, LogWindow& log) 
    : robotArm(robot), logWindow(log), isRunning(false), 
      stepMode(false), currentLine(0), mainThread(nullptr) {
    
    g_robotArm = &robot;
    InitLuaState();
}
void LuaController::InitLuaState() {
    if (mainThread) lua_close(mainThread);
    
    mainThread = luaL_newstate();
    luaL_openlibs(mainThread);
    L = mainThread;  // L będzie aktualnym wątkiem wykonawczym
    RegisterFunctions();
}


void LuaController::RegisterFunctions() {
    lua_register(L, "setJointRotation", lua_setJointRotation);
    lua_register(L, "wait", lua_wait);
    lua_register(L, "moveLinear", lua_moveLinear);
    lua_register(L, "moveParabolic", lua_moveParabolic); 
    lua_register(L, "moveSpline", lua_moveSpline);
    lua_register(L, "gripObject", lua_gripObject);
    lua_register(L, "releaseObject", lua_releaseObject);
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

int LuaController::lua_moveLinear(lua_State* L) {
    float x = lua_tonumber(L, 1);
    float y = lua_tonumber(L, 2);
    float z = lua_tonumber(L, 3);

    if(g_robotArm) {
        g_robotArm->MoveTo({x,y,z}, InterpolationType::LINEAR);
        last_wait = 1.0f; // Wait for animation
    }
    return lua_yield(L, 0);
}

int LuaController::lua_moveParabolic(lua_State* L) {
    float x = lua_tonumber(L, 1);
    float y = lua_tonumber(L, 2);
    float z = lua_tonumber(L, 3);

    if(g_robotArm) {
        g_robotArm->MoveTo({x,y,z}, InterpolationType::PARABOLIC);
        last_wait = 1.0f;
    }
    return lua_yield(L, 0);
}

int LuaController::lua_moveSpline(lua_State* L) {
    float x = lua_tonumber(L, 1);
    float y = lua_tonumber(L, 2);
    float z = lua_tonumber(L, 3);

    if(g_robotArm) {
        g_robotArm->MoveTo({x,y,z}, InterpolationType::SPLINE);
        last_wait = 1.0f;
    }
    return lua_yield(L, 0);
}

int LuaController::lua_gripObject(lua_State* L) {
    if(g_robotArm) {
        g_robotArm->GripObject();
        last_wait = 0.5f; // Krótkie opóźnienie na animację chwytu
    }
    return lua_yield(L, 0);
}

int LuaController::lua_releaseObject(lua_State* L) {
    if(g_robotArm) {
        g_robotArm->ReleaseObject();
        last_wait = 0.5f; // Krótkie opóźnienie na animację puszczenia
    }
    return lua_yield(L, 0);
}

LuaController::~LuaController() {
    if (mainThread) {
        lua_close(mainThread);
        mainThread = nullptr;
        L = nullptr;
    }
    g_robotArm = nullptr;
}

void LuaController::LoadScript(const std::string& code) {
    Stop();
    ResetLuaState();
    
    L = luaL_newstate();
    luaL_openlibs(L);ResetLuaState();
    lua_pushlightuserdata(L, this);
    lua_setfield(L, LUA_REGISTRYINDEX, "LuaController");
    
    // Ustaw hook debuggera
    lua_sethook(L, [](lua_State* L, lua_Debug* ar) {
        // Pobierz wskaźnik this z registry
        lua_getfield(L, LUA_REGISTRYINDEX, "LuaController");
        LuaController* controller = (LuaController*)lua_touserdata(L, -1);
        lua_pop(L, 1);  // Usuń wskaźnik ze stosu
        
        if(controller && ar->event == LUA_HOOKLINE) {
            lua_yield(L, 0);
        }
    }, LUA_MASKLINE, 0);
    
    if(luaL_loadstring(L, code.c_str()) != LUA_OK) {
        logWindow.AddLog(lua_tostring(L, -1), LogLevel::Error);
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
        ResetLuaState();
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
    lua_State* from = nullptr;
    int status = lua_resume(L, from, 0, &nresults);
    
    lua_Debug ar;
    if(lua_getstack(L, 0, &ar)) {
        lua_getinfo(L, "Snl", &ar);  
        std::string message = "Linia " + std::to_string(ar.currentline);
        logWindow.AddLog(message.c_str(), LogLevel::Info);
    }

    if(status != LUA_YIELD && status != LUA_OK) {
        std::string error = lua_tostring(L, -1);
        logWindow.AddLog(("Błąd wykonania: " + error).c_str(), LogLevel::Error);
        lua_pop(L, 1);
        Stop();
    }
    
    if(status == LUA_OK) {
        logWindow.AddLog("Skrypt zakończony", LogLevel::Info);
        Stop();
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

void LuaController::Update(float deltaTime) {
    if (!isRunning || stepMode) return;

    if (waitTime > 0) {
        waitTime -= deltaTime;
        return;
    }

    executeTimer += deltaTime;
    if (executeTimer >= EXECUTION_INTERVAL) {
        executeTimer = 0;
        
        int nresults;
        int status = lua_resume(L, nullptr, 0, &nresults);
        
        lua_Debug ar;
        if(lua_getstack(L, 0, &ar)) {
            lua_getinfo(L, "Snl", &ar);
            std::string message = "Wykonano krok";
            logWindow.AddLog(message.c_str(), LogLevel::Info);
        }

        if (status == LUA_YIELD) {
            // Ustaw czas oczekiwania z funkcji wait()
            waitTime = last_wait;
        }
        else if (status == LUA_OK) {
            logWindow.AddLog("Skrypt zakończony", LogLevel::Info);
            Stop();
        }
        else {
            std::string error = lua_tostring(L, -1);
            logWindow.AddLog(("Błąd wykonania: " + error).c_str(), LogLevel::Error);
            lua_pop(L, 1);
            Stop();
        }
    }
}