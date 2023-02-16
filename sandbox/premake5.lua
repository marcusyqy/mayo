project "sandbox"
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
        "%{include_dir.spdlog}",
        "%{include_dir.vulkan}",
        "%{include_dir.glm}",
        "%{include_dir.xtr}",
        "."
    }

    links {
        "%{library_dir.vulkan}"
    }

    warnings "Extra"

    --defines for msvc compiler
    filter "system:windows"
        kind "ConsoleApp"
        systemversion "latest"
        defines { "WIN32", "_CRT_SECURE_NO_WARNINGS" }

    filter "configurations:Debug"
        defines { "ZOO_ENABLE_LOGS" }
        runtime "Debug"
        symbols "on"
        links {
            "%{library_dir.shaderc_debug}"
        }

    filter "configurations:Release"
        runtime "Release"
        optimize "on"
        links {
            "%{library_dir.shaderc}"
        }

    filter "configurations:Dist"
        runtime "Release"
        optimize "on"
        links {
            "%{library_dir.shaderc}"
        }

