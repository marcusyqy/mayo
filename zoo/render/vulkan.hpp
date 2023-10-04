#include "core/window.hpp"
#include "fwd.hpp"

namespace zoo::vk {

static constexpr auto API_VERSION       = VK_API_VERSION_1_3;
static constexpr auto ENABLE_VALIDATION = true;

struct Render_Context {
    VkAllocationCallbacks allocation_callbacks{};

    VkInstance instance            = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT debug = VK_NULL_HANDLE;

    VkPhysicalDevice physical_device = VK_NULL_HANDLE;
    VkDevice device                  = VK_NULL_HANDLE;

    // @TODO: work towards supporting COMPUTE and PRESENT
    VkQueue graphics_queue = VK_NULL_HANDLE;
    VkQueue present_queue  = VK_NULL_HANDLE;

    VkSurfaceKHR surface = VK_NULL_HANDLE;
};

struct Dynamic_Data {};

Render_Context allocate_render_context(Window& window) noexcept;
void free_render_context(Render_Context& data) noexcept;

} // namespace zoo::vk
