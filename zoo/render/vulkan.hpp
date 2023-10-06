#include "core/window.hpp"
#include "fwd.hpp"

struct GLFWwindow;

namespace zoo::vk {

static constexpr auto API_VERSION       = VK_API_VERSION_1_3;
static constexpr auto ENABLE_VALIDATION = true;
enum : u32 { MAX_IMAGES = 3, MAX_WINDOWS = 5 };

// how to know when to deallocate? - use core/detail/window_registry to poll for deallocation.
struct Window_Data {
    GLFWwindow* context      = nullptr;
    VkSurfaceKHR surface     = VK_NULL_HANDLE;
    VkSwapchainKHR swapchain = VK_NULL_HANDLE;

    u32 width;
    u32 height;
    u32 current_idx;
};

struct Render_Context {
    VkAllocationCallbacks allocation_callbacks{};

    VkInstance instance            = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT debug = VK_NULL_HANDLE;

    VkPhysicalDevice physical_device = VK_NULL_HANDLE;
    VkDevice device                  = VK_NULL_HANDLE;

    // @TODO: work towards supporting COMPUTE and PRESENT
    VkQueue graphics_queue = VK_NULL_HANDLE;
    VkQueue present_queue  = VK_NULL_HANDLE;

    VkCommandPool graphics_command_pool = VK_NULL_HANDLE;
    VkCommandPool present_command_pool  = VK_NULL_HANDLE;

    // @TODO: make it dynamically grow.
    Window_Data windows[MAX_WINDOWS];
    u32 num_windows = {};
};

Render_Context allocate_render_context(Window& window) noexcept;
void free_render_context(Render_Context& data) noexcept;

void swap_buffer();

} // namespace zoo::vk
