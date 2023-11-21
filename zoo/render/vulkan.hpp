#include "core/window.hpp"
#include "fwd.hpp"

struct GLFWwindow;

namespace zoo::vk {

static constexpr auto API_VERSION       = VK_API_VERSION_1_3;
static constexpr auto ENABLE_VALIDATION = true;
enum : u32 { MAX_IMAGES = 3, MAX_WINDOWS = 5 };
constexpr static auto null_resource = std::numeric_limits<u32>::max();

//
// how to know when to deallocate? - use core/detail/window_registry to poll for deallocation.
struct Image {
    VkImage vk;
    VmaAllocation mem = VK_NULL_HANDLE; // will be null for swapchain images.
    u32 handle;
};

struct Window_Data {
    GLFWwindow* context      = nullptr;
    VkSurfaceKHR surface     = VK_NULL_HANDLE;
    VkSwapchainKHR swapchain = VK_NULL_HANDLE;

    u32 width;
    u32 height;
    u32 current_idx;
    u32 num_images;

    VkSurfaceFormatKHR surface_format;
    u32 images[MAX_IMAGES];
};


struct Image_View {
    VkImageView image_view;
    u32 handle;
};

template <typename T, u32 N>
struct Pool {
    u32 sparse[N];
    T direct[N];
    u32 objects_count = 0;
    u32 free_list     = null_resource;

    Pool() { memset(sparse, null_resource, N); }

    u32 create() {
        if (free_list != null_resource) {
            auto next = free_list;
            free_list = sparse[free_list];
            return next;
        }
        return objects_count++;
    }

    void erase(u32 index) {
        sparse[index] = free_list;
        free_list = index;
        --objects_count;
    }

    T& operator[](u32 index) { return direct[sparse[index]]; }
    const T& operator[](u32 index) const { return direct[sparse[index]]; }
    u32 count() const { return objects_count; }
};

struct Image_Registry {
    enum : u32 { MAX_IMAGE_VIEWS = 5, MAX_IMAGES = 100 };

    struct Node {
        Image image;
        Image_View image_views[MAX_IMAGE_VIEWS];
        u32 num_image_views = 0;
    };

    // VmaPool memory_pool; // for memory allocation.
    Pool<Node, MAX_IMAGES> pool;
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
    VmaAllocator allocator = VK_NULL_HANDLE;

    // @TODO: make it dynamically grow.
    Window_Data windows[MAX_WINDOWS];
    u32 num_windows = {};

    Image_Registry image_registry;
};

Render_Context allocate_render_context(Window& window) noexcept;
void free_render_context(Render_Context& data) noexcept;
void swap_buffer();

} // namespace zoo::vk
