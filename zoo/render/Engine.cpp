#include "render/Engine.hpp"
#include "core/Defines.hpp"
#include "core/Log.hpp"

#include <vulkan/vk_enum_string_helper.h>

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
        app_info.apiVersion = 0; // I don't know what is API Version
    }

    // TODO : still have to create instance properly but i'm really tired
    ZOO_LOG_ERROR("did not create this instance properly!!");

    VkInstanceCreateInfo create_info{};
    {
        create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        create_info.pNext = nullptr;
        create_info.flags = 0;
        create_info.pApplicationInfo = &app_info;

        create_info.enabledLayerCount;
        create_info.ppEnabledLayerNames;
        create_info.enabledExtensionCount;
        create_info.ppEnabledExtensionNames;
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

Engine::Engine(const engine::Info& info) : info_(info) {}

void Engine::initialize() noexcept {
    instance_ = create_instance(info_);
    device_ = create_device(instance_);
}

void Engine::cleanup() noexcept {
    if (instance_ != nullptr) {
        vkDestroyInstance(instance_, nullptr);
    }
}

} // namespace zoo::render