#pragma once
#include "fwd.hpp"
#include "stdx/span.hpp"

namespace zoo::render {

class shader {
public:
    shader([[maybe_unused]] stdx::span<char> code) {}

private:
    VkShaderModule module_;
};

class pipeline {
public:
private:
};
} // namespace zoo::render
