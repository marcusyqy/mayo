
group "Dependencies"
warnings "Off"
    include "glfw"
    include "vma"
    include "tinyobj"
    include "stb"
    project "imgui"
        language "C++"
        cppdialect "C++17"
        staticruntime "on"
        characterset "MBCS"
        kind "StaticLib"
        targetdir ("bin/" .. outputdir .. "/%{prj.name}")
        objdir ("bin-int/" .. outputdir .. "/%{prj.name}")
        files {
            "%{prj.name}/*.hpp",
            "%{prj.name}/*.cpp",
            "%{prj.name}/*.h"
        }
        includedirs {
            "%{include_dir.GLFW}",
            "%{include_dir.vulkan}",
            "%{prj.name}"
        }

        links {
            "GLFW",
            "%{library_dir.vulkan}"
        }

        filter "system:linux"
            pic "On"

        filter "configurations:Debug"
            runtime "Debug"
            symbols "on"

        filter "configurations:Release"
            runtime "Release"
            optimize "on"

        filter "configurations:Dist"
            runtime "Release"
            optimize "on"
            symbols "off"

