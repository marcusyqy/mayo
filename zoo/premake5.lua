project "zoo"
    language "C++"
    cppdialect "C++17"
    staticruntime "on"
    characterset "MBCS"
    targetdir("bin/" .. outputdir .. "/%{prj.name}")
    objdir("bin-int/" .. outputdir .. "/%{prj.name}")

    --recursively add every cpp/h/hpp file into the project
    files {
        "**.cpp",
        "**.hpp",
        "**.h"
    }

    includedirs {
        "%{include_dir.GLFW}",
        "%{include_dir.spdlog}",
        "%{include_dir.xtr}",
        "%{include_dir.vulkan}",
        "%{include_dir.glm}",
        "."
    }

    links {
        "GLFW",
        "vma",
        "%{library_dir.vulkan}"
    }

    warnings "Extra"

    --defines for msvc compiler
    filter "system:windows"
    kind "ConsoleApp"
    systemversion "latest"
    defines { "WIN32", "_CRT_SECURE_NO_WARNINGS" }

    filter "system:linux"
    kind "ConsoleApp"
    systemversion "latest"
    defines {}

    filter "configurations:Debug"
        defines { "ZOO_ENABLE_LOGS" }
        runtime "Debug"
        symbols "on"
        links {
            "%{library_dir.shaderc_debug}"
        }

    filter "configurations:Release"
        defines { "ZOO_ENABLE_LOGS" }
        runtime "Release"
        optimize "on"
        links {
            "%{library_dir.shaderc}"
        }

    filter "configurations:Dist"
        defines { "ZOO_DIST" }
        runtime "Release"
        optimize "on"
        links {
            "%{library_dir.shaderc}"
        }

