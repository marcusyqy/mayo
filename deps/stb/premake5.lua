project "stb"
    language "C++"
    cppdialect "C++17"
    staticruntime "on"
    characterset "MBCS"
    targetdir("bin/" .. outputdir .. "/%{prj.name}")
    objdir("bin-int/" .. outputdir .. "/%{prj.name}")
    kind "StaticLib"
    warnings "Extra"
    files {
        "**.hpp",
        "**.cpp",
        "**.h"
    }
    filter "configurations:Debug"
        runtime "Debug"
        symbols "on"

    filter "configurations:Release"
        runtime "Release"
        optimize "on"

    filter "configurations:Dist"
        runtime "Release"
        optimize "on"
