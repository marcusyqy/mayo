#include "render/Engine.hpp"
#include "core/Log.hpp"
#include "core/fwd.hpp"

#include "Defines.hpp"

#include "core/platform/Query.hpp"
#include "render/fwd.hpp"

#include <optional>

namespace zoo::render {

namespace {

platform::render::Parameters params{ true };
platform::render::Query query{ params };

std::optional<size_t> get_queue_index_if_physical_device_is_chosen(
    const render::utils::PhysicalDevice& physical_device,
    VkInstance instance) noexcept {

    if (!physical_device.has_geometry_shader() ||
        !physical_device.has_required_extension(VK_KHR_SWAPCHAIN_EXTENSION_NAME) ||
        !physical_device.shader_draw_parameters_enabled()) {
        return std::nullopt;
    }

    size_t index{};
    for (const auto& queue_properties : physical_device.queue_properties()) {
        if (queue_properties.has_graphics() && physical_device.has_present(queue_properties, instance))
            return { index };
        ++index;
    }
    return std::nullopt;
}

uint32_t make_version(core::Version version) noexcept {
    return VK_MAKE_VERSION(version.major, version.minor, version.patch);
}

VkInstance create_instance(const engine::Info& info) noexcept {
    VkApplicationInfo app_info{ .sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO,
                                .pNext              = nullptr, // for now
                                .pApplicationName   = info.app_info.name.c_str(),
                                .applicationVersion = make_version(info.app_info.version),
                                .pEngineName        = core::engine::name.data(),
                                .engineVersion      = make_version(core::engine::version),
                                .apiVersion         = Defines::vk_version };

    platform::render::Info query_info = query.get_info();
    const auto& enabled_layers        = query_info.layers_;
    const auto& enabled_extensions    = query_info.extensions_;

    VkInstanceCreateInfo create_info{
        .sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext                   = nullptr,
        .flags                   = 0,
        .pApplicationInfo        = &app_info,
        .enabledLayerCount       = static_cast<u32>(enabled_layers.size()),
        .ppEnabledLayerNames     = enabled_layers.data(),
        .enabledExtensionCount   = static_cast<u32>(enabled_extensions.size()),
        .ppEnabledExtensionNames = enabled_extensions.data(),
    };

    VkInstance instance = nullptr;
    VK_EXPECT_SUCCESS(vkCreateInstance(&create_info, nullptr, &instance));
    return instance;
}

std::vector<utils::PhysicalDevice> populate_physical_devices(VkInstance instance) noexcept {
    if (instance == nullptr) return {};

    u32 device_count = 0;
    VK_EXPECT_SUCCESS(vkEnumeratePhysicalDevices(instance, &device_count, nullptr));

    if (device_count == 0) {
        ZOO_LOG_ERROR("Devices cannot be 0 for vkEnumeratePhysicalDevices!");

        // TODO: this needs to fail (assertion) or some sort of termination that
        // tells us something meaningful which users can report to us about.
        return {};
    }

    std::vector<VkPhysicalDevice> devices(device_count);
    VK_EXPECT_SUCCESS(vkEnumeratePhysicalDevices(instance, &device_count, devices.data()));

    return { std::begin(devices), std::end(devices) };
}

std::optional<debug::Messenger> create_debugger(VkInstance instance, const engine::Info& info) noexcept {
    return instance != nullptr && info.debug_layer ? std::make_optional(instance) : std::nullopt;
}

DeviceContext create_context(VkInstance instance, const std::vector<utils::PhysicalDevice>& physical_devices) {

    for (const auto& pd : physical_devices) {
        auto optional_index = get_queue_index_if_physical_device_is_chosen(pd, instance);

        if (optional_index) {
            return { instance, pd, pd.queue_properties()[*optional_index], query };
        }
    }

    ZOO_ASSERT(false, "Something went wrong when choosing physical devices");
    // default to first device.
    return { instance, physical_devices.front(), physical_devices.front().queue_properties()[0], query };
}

} // namespace

Engine::Engine(const Info& info) noexcept :
    info_(info), instance_(create_instance(info_)), physical_devices_(populate_physical_devices(instance_)),
    context_(create_context(instance_, physical_devices_)), debugger_(create_debugger(instance_, info)) {}

Engine::~Engine() noexcept {
    debugger_.reset();
    context_.reset();
    if (instance_ != nullptr) {
        instance_ = nullptr;
        vkDestroyInstance(instance_, nullptr);
    }
}

} // namespace zoo::render
