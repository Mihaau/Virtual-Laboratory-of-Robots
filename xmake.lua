add_rules("mode.debug", "mode.release")
add_requires("raylib", "nlohmann_json", "lua")
add_requires("imgui docking", {configs = {glfw = true, opengl3 = true, docking = true}})
add_rules("plugin.compile_commands.autoupdate", {outputdir = ".vscode"})
set_languages("c++20")
target("robolab")
    set_policy("run.autobuild", true)
    set_kind("binary")
    add_files("src/*.cpp")
    add_includedirs("include")
    add_packages("raylib","imgui docking", "imgui", "nlohmann_json", "lua")
    after_build(function (target)
        local targetdir = target:targetdir()
        os.cp("$(projectdir)/assets",targetdir)
    end)
    before_build(function (target)
        os.rm("$(projectdir)/build")
    end)