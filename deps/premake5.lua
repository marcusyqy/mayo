
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
            "%{prj.name}/*.h",
            "%{prj.name}/backends/imgui_impl_glfw.h",
            "%{prj.name}/backends/imgui_impl_glfw.cpp",
            "%{prj.name}/backends/imgui_impl_opengl3.h",
            "%{prj.name}/backends/imgui_impl_opengl3.cpp",
            "%{prj.name}/backends/imgui_impl_win32.h",
            "%{prj.name}/backends/imgui_impl_win32.cpp",
            "%{prj.name}/backends/imgui_impl_dx11.h",
            "%{prj.name}/backends/imgui_impl_dx11.cpp"
        }

        includedirs {
            "%{include_dir.GLFW}",
            -- "%{include_dir.vulkan}",
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

    project "glad"
        language "C"
        staticruntime "on"
        characterset "MBCS"
        kind "StaticLib"
        targetdir ("bin/" .. outputdir .. "/%{prj.name}")
        objdir ("bin-int/" .. outputdir .. "/%{prj.name}")
        files {
            "%{prj.name}/*.h",
            "%{prj.name}/*.c",
        }
        includedirs {
            "%{include_dir.GLFW}",
            "%{include_dir.vulkan}",
            "%{prj.name}"
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
