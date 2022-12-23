#include "render/engine.hpp"
#include "core/fwd.hpp"
#include "core/log.hpp"

#include "core/platform/query.hpp"
#include <vulkan/vk_enum_string_helper.h>

#include <optional>

#include "vulkan/utils/physical_device_scorer.hpp"

namespace zoo::render {

namespace {

uint32_t make_version(core::version version) noexcept {
    return VK_MAKE_VERSION(version.major, version.minor, version.patch);
}

VkInstance create_instance(const engine::info& info) noexcept {
    VkApplicationInfo app_info{};
    {
        app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        app_info.pNext = nullptr; // for now
        app_info.pApplicationName = info.app_info.name.c_str();
        app_info.applicationVersion = make_version(info.app_info.version);
        app_info.pEngineName = core::engine::name.data();
        app_info.engineVersion = make_version(core::engine::version);
        app_info.apiVersion = VK_API_VERSION_1_3;
    }

    VkInstanceCreateInfo create_info{};

    platform::render::parameters params{true};
    platform::render::query query{params};

    platform::render::info vulkan_query_info = query.get_info();
    const auto& enabled_layers = vulkan_query_info.layers_;
    const auto& enabled_extensions = vulkan_query_info.extensions_;
    {
        create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        create_info.pNext = nullptr;
        create_info.flags = 0;
        create_info.pApplicationInfo = &app_info;

        create_info.enabledLayerCount =
            static_cast<uint32_t>(enabled_layers.size());
        create_info.ppEnabledLayerNames = enabled_layers.data();
        create_info.enabledExtensionCount =
            static_cast<uint32_t>(enabled_extensions.size());
        create_info.ppEnabledExtensionNames = enabled_extensions.data();
    }

    VkInstance instance = nullptr;
    VkResult result = vkCreateInstance(&create_info, nullptr, &instance);
    if (result != VK_SUCCESS)
        ZOO_LOG_ERROR("vkCreateInstance failed with exit code = {}",
            string_VkResult(result));

    return instance;
}

std::vector<vulkan::utils::physical_device> populate_physical_devices(
    VkInstance instance) noexcept {
    if (instance == nullptr)
        return {};

    uint32_t device_count = 0;
    VkResult result =
        vkEnumeratePhysicalDevices(instance, &device_count, nullptr);

    if (result != VK_SUCCESS || device_count == 0) {
        ZOO_LOG_ERROR("Devices cannot be 0 for vkEnumeratePhysicalDevices!");
        return {};
    }

    std::vector<VkPhysicalDevice> devices(device_count);
    vkEnumeratePhysicalDevices(instance, &device_count, devices.data());
    return {std::begin(devices), std::end(devices)};
}
//    std::vector<physical_device_scorer> scorers{
//        std::begin(devices), std::end(devices)};
//    auto chosen = std::max_element(std::begin(scorers), std::end(scorers));
//    const auto index = std::distance(scorers.begin(), chosen);
//    return std::make_shared<vulkan::device>(instance, devices[index]);
//}

} // namespace

engine::engine(const engine::info& info) noexcept : info_(info) {}

engine::~engine() noexcept { cleanup(); }

void engine::initialize() noexcept {
    instance_ = create_instance(info_);
    if (instance_ != nullptr && info_.debug_layer)
        debugger_ = std::make_optional<vulkan::debug::messenger>(instance_);
    physical_devices_ = populate_physical_devices(instance_);
}

void engine::cleanup() noexcept {
    devices_.clear();
    debugger_.reset();

    if (instance_ != nullptr) {
        instance_ = nullptr;
        vkDestroyInstance(instance_, nullptr);
    }
}

} // namespace zoo::render
