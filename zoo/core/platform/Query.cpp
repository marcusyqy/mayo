#include "Query.hpp"
#include <cstring>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iterator>

namespace zoo::platform::render {

Query::Query(Parameters parameters) noexcept : parameters_{ parameters } {}

Info Query::get_info() const noexcept {
    Info info{};
    info.extensions_ = get_extensions();
    info.layers_     = get_layers();
    return info;
}

std::vector<const char*> Query::get_extensions() const noexcept {
    std::vector<const char*> extensions{};
    uint32_t glfw_extensions_count;
    const char** glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extensions_count);

    std::copy(glfw_extensions, glfw_extensions + glfw_extensions_count, std::back_inserter(extensions));

    if (parameters_.validation_) {
        extensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
}

std::vector<const char*> Query::get_layers() const noexcept {
    if (parameters_.validation_) {
        const char* validation_layer{ "VK_LAYER_KHRONOS_validation" };
        uint32_t layer_count;
        vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
        std::vector<VkLayerProperties> available_layers(static_cast<size_t>(layer_count));
        vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());

        for (const auto& available_layer : available_layers) {
            if (strcmp(validation_layer, available_layer.layerName)) {
                return std::vector<const char*>{ validation_layer };
            }
        }
    }

    return std::vector<const char*>();
}

const Parameters& Query::get_params() const noexcept { return parameters_; }

} // namespace zoo::platform::render
