project "imgui"
    language "C++"
    cppdialect "C++17"
    staticruntime "on"
    characterset "MBCS"
    kind "StaticLib"
    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")
    files {
        "%{prj.name}/backends/imgui_impl_glfw.h",
        "%{prj.name}/backends/imgui_impl_glfw.cpp",
        "%{prj.name}/backends/imgui_impl_vulkan.h",
        "%{prj.name}/backends/imgui_impl_vulkan.cpp",
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
