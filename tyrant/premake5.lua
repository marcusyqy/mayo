project "tyrant"
    language "C++"
    cppdialect "C++20"
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
        "%{include_dir.imgui}",
        "%{include_dir.spdlog}",
        "%{include_dir.vma}",
        "%{include_dir.tinyobj}",
        "%{include_dir.stb}",
        "%{include_dir.mods}",
        "%{include_dir.vulkan}",
        "%{include_dir.glm}",
        "%{include_dir.mayo_core}",
        "."
    }

    links {
        "GLFW",
        "vma",
        "%{library_dir.vulkan}",
        "tinyobj",
        "stb",
        "imgui",
        "glad"
    }

    warnings "Extra"

    --defines for msvc compiler
    filter "system:windows"
    kind "ConsoleApp" -- "WindowedApp"
    systemversion "latest"
    defines { "WIN32", "_CRT_SECURE_NO_WARNINGS" }

    -- this shouldn't be used anymore since we're not using opengl?
    filter "system:linux"
    kind "ConsoleApp"
    systemversion "latest"
    defines {}

    filter "configurations:Debug"
        defines { "TYRANT_ENABLE_LOGS" }
        runtime "Debug"
        symbols "on"
        links {
            "%{library_dir.shaderc_debug}"
        }

    filter "configurations:Release"
        defines { "TYRANT_ENABLE_LOGS" }
        runtime "Release"
        optimize "on"
        links {
            "%{library_dir.shaderc}"
        }

    filter "configurations:Dist"
        defines { "TYRANT_DIST" }
        runtime "Release"
        optimize "on"
        links {
            "%{library_dir.shaderc}"
        }

