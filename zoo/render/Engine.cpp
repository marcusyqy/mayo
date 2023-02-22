#include "render/Engine.hpp"
#include "core/Log.hpp"
#include "core/fwd.hpp"

#include "core/platform/Query.hpp"
#include "render/fwd.hpp"

#include <optional>

namespace zoo::render {

namespace {

platform::render::Parameters params{true};
platform::render::Query query{params};

std::optional<size_t> get_queue_index_if_physical_device_is_chosen(
    const render::utils::PhysicalDevice& physical_device,
    VkInstance instance) noexcept {
    if (!physical_device.has_geometry_shader() &&
        physical_device.has_required_extension(VK_KHR_SWAPCHAIN_EXTENSION_NAME))
        return std::nullopt;

    size_t index{};
    for (const auto& queue_properties : physical_device.queue_properties()) {
        if (queue_properties.has_graphics() &&
            physical_device.has_present(queue_properties, instance))
            return std::make_optional(index);
        ++index;
    }
    return std::nullopt;
}

uint32_t make_version(core::Version version) noexcept {
    return VK_MAKE_VERSION(version.major, version.minor, version.patch);
}

VkInstance create_instance(const engine::Info& info) noexcept {
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

    platform::render::info query_info = query.get_info();
    const auto& enabled_layers = query_info.layers_;
    const auto& enabled_extensions = query_info.extensions_;
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
    VK_EXPECT_SUCCESS(vkCreateInstance(&create_info, nullptr, &instance));
    return instance;
}

std::vector<utils::PhysicalDevice> populate_physical_devices(
    VkInstance instance) noexcept {
    if (instance == nullptr)
        return {};

    uint32_t device_count = 0;
    VK_EXPECT_SUCCESS(
        vkEnumeratePhysicalDevices(instance, &device_count, nullptr));

    if (device_count == 0) {
        ZOO_LOG_ERROR("Devices cannot be 0 for vkEnumeratePhysicalDevices!");
        // TODO: this needs to fail (assertion) or some sort of termination that
        // tells us something meaningful which users can report to us about.
        return {};
    }

    std::vector<VkPhysicalDevice> devices(device_count);
    VK_EXPECT_SUCCESS(
        vkEnumeratePhysicalDevices(instance, &device_count, devices.data()));

    return {std::begin(devices), std::end(devices)};
}
//    std::vector<physical_device_scorer> scorers{
//        std::begin(devices), std::end(devices)};
//    auto chosen = std::max_element(std::begin(scorers), std::end(scorers));
//    const auto index = std::distance(scorers.begin(), chosen);
//    return std::make_shared<device>(instance, devices[index]);
//}

} // namespace

Engine::Engine(const Info& info) noexcept : info_(info) { initialize(); }

Engine::~Engine() noexcept { cleanup(); }

void Engine::initialize() noexcept {
    instance_ = create_instance(info_);
    if (instance_ != nullptr && info_.debug_layer)
        debugger_ = std::make_optional<debug::Messenger>(instance_);
    physical_devices_ = populate_physical_devices(instance_);

    for (const auto& pd : physical_devices_) {
        auto optional_index =
            get_queue_index_if_physical_device_is_chosen(pd, instance_);
        if (optional_index) {
            context_ = std::make_shared<DeviceContext>(
                instance_, pd, pd.queue_properties()[*optional_index], query);
            break;
        }
    }
}

void Engine::cleanup() noexcept {
    debugger_.reset();
    context_.reset();

    if (instance_ != nullptr) {
        instance_ = nullptr;
        vkDestroyInstance(instance_, nullptr);
    }
}

} // namespace zoo::render
