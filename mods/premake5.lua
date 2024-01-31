group "mods"

project "stdx"
    language "C++"
    cppdialect "C++17"
    staticruntime "on"
    characterset "MBCS"
    kind "None"
    files {
        "%{prj.name}/**.hpp",
        "%{prj.name}/**.cpp",
        "%{prj.name}/**.h"
    }

