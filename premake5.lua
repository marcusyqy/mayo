require("lua/plugs")
require("lua/globals")

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

--for grouping projects in the future
group "Dependencies"
warnings "Off"
    include "deps/glfw"
    include "deps/vma"
    include "deps/tinyobj"
    include "xtr"


group ""
    include "zoo"
    include "sandbox"

-- -- to make premake behave like cmake
-- postbuildmessage "Regenerating project files with Premake5!"
-- postbuildcommands
-- {
--     "\"%{prj.location}bin/premake5\" %{_ACTION} --file=\"%{wks.location}premake5.lua\""
-- }
