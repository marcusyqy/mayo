#pragma once
#include "types.hpp"

#ifdef WIN32
#include <windows.h>
#undef max
#endif

#include <vulkan/vulkan.h>

struct Render_Params {
    enum : u32 {
        MAX_SWAPCHAIN_IMAGES   = 3,
        MAX_NUMBER_DEVICES     = 10,
        MAX_FORMAT_COUNT       = 100,
        MAX_PRESENT_MODE_COUNT = 100
    };
};

struct Swapchain {
    VkSurfaceKHR surface      = { VK_NULL_HANDLE };
    VkSwapchainKHR handle     = { VK_NULL_HANDLE };
    VkSurfaceFormatKHR format = {};
    u32 num_images            = {};
    u32 current_frame         = {};
    u32 out_of_sync_index     = {};
    bool out_of_date          = {};

    u32 width = {}, height = {};

    VkImageView image_views[Render_Params::MAX_SWAPCHAIN_IMAGES] = {};
    VkSemaphore image_avail[Render_Params::MAX_SWAPCHAIN_IMAGES] = {};
    VkSemaphore render_done[Render_Params::MAX_SWAPCHAIN_IMAGES] = {};
    VkFence in_flight[Render_Params::MAX_SWAPCHAIN_IMAGES]       = {};
};

struct Command {
    VkFence fence;
    VkCommandBuffer command_buffer;
};


// @TODO: TBD
struct Draw_Command : Command {
    VkBuffer buffer; // maybe
};

//
void init_vulkan_resources();
void free_vulkan_resources();

// swapchain
void free_swapchain(Swapchain& swap_chain);
void resize_swapchain(Swapchain& swapchain, u32 width, u32 height);
void present_swapchain(Swapchain& swapchain);

// shader
VkShaderModule create_shader();

#ifdef WIN32

Swapchain create_swapchain_from_win32(HINSTANCE hinstance, HWND hwnd);

#endif // WIN32
