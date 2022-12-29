#include "render/engine.hpp"
#include "core/fwd.hpp"
#include "core/log.hpp"

#include "core/platform/query.hpp"
#include "render/fwd.hpp"

#include <optional>

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

std::vector<utils::physical_device> populate_physical_devices(
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

engine::engine(const engine::info& info) noexcept : info_(info) {}

engine::~engine() noexcept { cleanup(); }

void engine::initialize() noexcept {
    instance_ = create_instance(info_);
    if (instance_ != nullptr && info_.debug_layer)
        debugger_ = std::make_optional<debug::messenger>(instance_);
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

std::shared_ptr<device> engine::promote(
    physical_device_iterator physical_device,
    utils::queue_family_properties family_props) noexcept {
    // TODO: get a better query parameter.
    platform::render::parameters params{true};
    platform::render::query query{params};
    devices_.push_back(std::make_shared<device>(
        instance_, *physical_device, family_props, query));
    physical_devices_.erase(physical_device);
    return devices_.back();
}
} // namespace zoo::render
