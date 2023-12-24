require("lua/plugs")
require("lua/globals")

workspace "mayo"
architecture "x86_64"
startproject "tyrant"

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
include "deps"
include "mods" -- this should be combined with `deps` potentially.

group ""
    include "zoo"
    include "tyrant"
    include "sandbox"

-- -- to make premake behave like cmake
-- postbuildmessage "Regenerating project files with Premake5!"
-- postbuildcommands
-- {
--     "\"%{prj.location}bin/premake5\" %{_ACTION} --file=\"%{wks.location}premake5.lua\""
-- }
