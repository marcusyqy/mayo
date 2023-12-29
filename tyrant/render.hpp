#pragma once
#include "types.hpp"

#ifdef WIN32
#include <windows.h>
#undef max
#endif

#include <vulkan/vulkan.h>

struct Swapchain {
    VkSurfaceKHR surface      = { VK_NULL_HANDLE };
    VkSwapchainKHR handle     = { VK_NULL_HANDLE };
    VkSurfaceFormatKHR format = {};
    u32 num_images            = {};

    u32 width = {}, height = {};
};

//
void init_vulkan_resources();
void free_vulkan_resources();

#ifdef WIN32
Swapchain create_swapchain_from_win32(HINSTANCE hinstance, HWND hwnd);
#endif

void free_swapchain(Swapchain& swap_chain);

