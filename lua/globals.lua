
VULKAN_SDK = os.getenv("VULKAN_SDK")

-- Include directories relative to root folder (solution directory)
include_dir = {}
include_dir["GLFW"] = "%{wks.location}/deps/glfw/include"
include_dir["vma"] = "%{wks.location}/deps/vma"
include_dir["tinyobj"] = "%{wks.location}/deps/tinyobj"
include_dir["spdlog"] = "%{wks.location}/deps/spdlog/include"
include_dir["xtr"] = "%{wks.location}/xtr"
include_dir["glm"] = "%{wks.location}/deps/glm"
include_dir["vulkan"] = "%{VULKAN_SDK}/Include"


library_dir = {}
library_dir["vulkan"] = "%{VULKAN_SDK}/Lib/vulkan-1.lib"
library_dir["shaderc"] = "%{VULKAN_SDK}/Lib/shaderc_shared.lib"
library_dir["shaderc_debug"] = "%{VULKAN_SDK}/Lib/shaderc_sharedd.lib"
