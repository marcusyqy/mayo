#pragma once
#include "core/log.hpp"
#include <vulkan/vulkan.h>

namespace zoo::render {

// TODO: maybe std::exit is not the way to go (change this later on)
#define VK_EXPECT_SUCCESS(EXP)                                                 \
    if (EXP != VK_SUCCESS) {                                                   \
        ZOO_LOG_ERROR("VK_SUCCESS NOT MET FOR CALL : " #EXP);                  \
        std::exit(-1);                                                         \
    }

} // namespace zoo::render
