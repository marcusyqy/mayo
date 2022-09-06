#include "render/Engine.hpp"
#include "core/Defines.hpp"
#include "core/Log.hpp"

#include <vulkan/vk_enum_string_helper.h>
#include "core/platform/Query.hpp"

namespace zoo::render {

namespace {
uint32_t make_version(core::Version version) noexcept {
    return VK_MAKE_VERSION(version.major_, version.minor_, version.patch_);
}

VkInstance create_instance(const engine::Info& info) noexcept {
    VkApplicationInfo app_info{};
    {
        app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        app_info.pNext = nullptr; // for now
        app_info.pApplicationName = info.app_info_.name_.c_str();
        app_info.applicationVersion = make_version(info.app_info_.version_);
        app_info.pEngineName = core::engine::name.data();
        app_info.engineVersion = make_version(core::engine::version);
        app_info.apiVersion = VK_API_VERSION_1_3;
    }

    VkInstanceCreateInfo create_info{};

    zoo::platform::vulkan::Parameters params{true};
    zoo::platform::vulkan::Query query{params};

    zoo::platform::vulkan::Info vulkan_query_info = query.get_info();
    const auto& enabled_layers = vulkan_query_info.layers_;
    const auto& enabled_extensions = vulkan_query_info.extensions_;
    {
        create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        create_info.pNext = nullptr;
        create_info.flags = 0;
        create_info.pApplicationInfo = &app_info;

        create_info.enabledLayerCount = static_cast<uint32_t>(enabled_layers.size());
        create_info.ppEnabledLayerNames = enabled_layers.data();
        create_info.enabledExtensionCount = static_cast<uint32_t>(enabled_extensions.size());;
        create_info.ppEnabledExtensionNames = enabled_extensions.data();
    }

    VkInstance instance = VK_NULL_HANDLE;
    VkResult result = vkCreateInstance(&create_info, nullptr, &instance);
    if (result != VK_SUCCESS)
        ZOO_LOG_ERROR("vkCreateInstance failed with exit code = {}",
            string_VkResult(result));

    return instance;
}

std::shared_ptr<vulkan::Device> create_device(VkInstance instance) noexcept {
    if (instance == VK_NULL_HANDLE)
        return nullptr;
    return std::make_shared<vulkan::Device>(instance);
}

} // namespace

Engine::Engine(const engine::Info& info) noexcept : info_(info) {}

Engine::~Engine() noexcept { cleanup(); }

void Engine::initialize() noexcept {
    instance_ = create_instance(info_);
    if(instance_ != VK_NULL_HANDLE && info_.debug_layer_)
        debugger_ = std::make_optional<vulkan::debug::Messenger>(instance_);
    device_ = create_device(instance_);
}

void Engine::cleanup() noexcept {
    device_.reset();
    debugger_.reset();
    if (instance_ != VK_NULL_HANDLE) {
        instance_ = VK_NULL_HANDLE;
        vkDestroyInstance(instance_, nullptr);
    }
}

} // namespace zoo::render