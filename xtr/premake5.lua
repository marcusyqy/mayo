group "xtr"

project "refl"
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

project "vma"
    language "C++"
    cppdialect "C++17"
    staticruntime "on"
    characterset "MBCS"
    targetdir("bin/" .. outputdir .. "/%{prj.name}")
    objdir("bin-int/" .. outputdir .. "/%{prj.name}")
    kind "StaticLib"
    warnings "Extra"
    includedirs {
        "%{include_dir.vulkan}"
    }
    links {
        "%{library_dir.vulkan}"
    }
    files {
        "%{prj.name}/**.hpp",
        "%{prj.name}/**.cpp",
        "%{prj.name}/**.h"
    }
