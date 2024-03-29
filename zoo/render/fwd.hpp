#pragma once
#include "core/fwd.hpp"
#include "core/log.hpp"
#include "stdx/span.hpp"
#include "vma/vk_mem_alloc.h"
#include <vulkan/vk_enum_string_helper.h>
#include <vulkan/vulkan.h>

namespace zoo::render::hidden::detail {

void maybe_invoke(VkResult result) noexcept;

template <typename Call, typename... Args>
void maybe_invoke(VkResult result, Call then, Args&&... args) noexcept {
    then(result, std::forward<Args>(args)...);
}

} // namespace zoo::render::hidden::detail

// TODO: maybe std::exit is not the way to go (change this later on)
#define VK_EXPECT_SUCCESS(EXP, ...)                                                                                    \
    if (VkResult ____result = EXP; ____result != VK_SUCCESS) {                                                         \
        ZOO_LOG_ERROR(                                                                                                 \
            "VK_SUCCESS NOT MET FOR CALL : " #EXP " , failed with exit code = {}",                                     \
            string_VkResult(____result));                                                                              \
        zoo::render::hidden::detail::maybe_invoke(____result, __VA_ARGS__);                                            \
    }

namespace zoo::render {

struct ViewportInfo {
    VkViewport viewport;
    VkRect2D scissor;
};

// @TODO: find somewhere else to put this.
template <typename T>
struct V2 {
    T x = {};
    T y = {};
};

struct RenderArea {
    V2<s32> offset;
    V2<u32> extent;
};

/*
 * Dictates the type of queue being used
 * */
enum class Operation : uint32_t { graphics, compute, transfer, present, unknown };

class Device_Context;
} // namespace zoo::render
