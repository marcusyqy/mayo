#pragma once
#include "core/log.hpp"
#include <vulkan/vk_enum_string_helper.h>
#include <vulkan/vulkan.h>

namespace zoo::render::hidden::detail {

void maybe_invoke(VkResult result) noexcept;

template<typename Call, typename... Args>
void maybe_invoke(VkResult result, Call then, Args&&... args) noexcept {
    then(result, std::forward<Args>(args)...);
    maybe_invoke(result);
}

} // namespace zoo::render::hidden::detail

// TODO: maybe std::exit is not the way to go (change this later on)
#define VK_EXPECT_SUCCESS(EXP, ...)                                            \
    if (VkResult result = EXP; result != VK_SUCCESS) {                         \
        ZOO_LOG_ERROR("VK_SUCCESS NOT MET FOR CALL : " #EXP                    \
                      " , failed with exit code = {}",                         \
            string_VkResult(result));                                          \
        zoo::render::hidden::detail::maybe_invoke(result, __VA_ARGS__);        \
    }
