require("deps/export-compile-commands")

workspace "zoo"
architecture "x86_64"
startproject "zoo"

configurations {
    "Debug",
    "Release",
    "Dist"
}

flags {
    "MultiProcessorCompile"
}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

VULKAN_SDK = os.getenv("VULKAN_SDK")
-- Include directories relative to root folder (solution directory)
include_dir = {}
include_dir["GLFW"] = "%{wks.location}/deps/glfw/include"
include_dir["spdlog"] = "%{wks.location}/deps/spdlog/include"
include_dir["vulkan"] =  "%{VULKAN_SDK}/Include"

--for grouping projects in the future
group "Dependencies"
warnings "Off"
include "deps/glfw"

group ""
--start of engine project
project "zoo"
location "zoo"
language "C++"
cppdialect "C++20"
staticruntime "on"
characterset "MBCS"
targetdir ("bin/" .. outputdir .. "/%{prj.name}")
objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

--recursively add every cpp/h/hpp file into the project
files {
    "%{prj.name}/**.cpp",
    "%{prj.name}/**.hpp",
    "%{prj.name}/**.h"
}

includedirs {
    "%{include_dir.GLFW}",
    "%{include_dir.spdlog}",
    "%{include_dir.vulkan}",
    "%{prj.name}"
}

links {
    "GLFW",
    "%{VULKAN_SDK}/Lib/vulkan-1.lib"
}

warnings "Extra"

--defines for msvc compiler
filter "system:windows"
    kind "ConsoleApp"
    systemversion "latest"
    defines { "WIN32", "_CRT_SECURE_NO_WARNINGS" }

filter "configurations:Debug"
    runtime "Debug"
    symbols "on"

filter "configurations:Release"
    runtime "Release"
    optimize "on"

filter "configurations:Dist"
    defines { "ZOO_DIST" }
    runtime "Release"
    optimize "on"


-- -- to make premake behave like cmake
-- postbuildmessage "Regenerating project files with Premake5!"
-- postbuildcommands
-- {
--     "\"%{prj.location}bin/premake5\" %{_ACTION} --file=\"%{wks.location}premake5.lua\""
-- }
