#pragma once
#include "device_context.hpp"
#include "fwd.hpp"
#include "stdx/span.hpp"

namespace zoo::render {

class shader {
public:
    using underlying_type = VkShaderModule;

    operator underlying_type() const noexcept { return get(); }
    underlying_type get() const noexcept { return module_; }

    std::string_view entry_point() const noexcept { return entry_point_; }

    void reset() noexcept;

    shader(std::shared_ptr<device_context> context, stdx::span<char> code,
        std::string_view entry_point) noexcept;

    shader() noexcept;
    ~shader() noexcept;

    shader(const shader& other) = delete;
    shader& operator=(const shader& other) = delete;

    shader(shader&& other) noexcept;
    shader& operator=(shader&& other) noexcept;

private:
    std::shared_ptr<device_context> context_;
    underlying_type module_ = nullptr;
    std::string entry_point_;
};

struct shader_stages_specifications {
    const shader& vertex;
    const shader& fragment;
};

struct viewport_info {
    VkViewport viewport;
    VkRect2D scissor;
};

class pipeline {
public:
    using underlying_type = VkPipeline;

    pipeline(std::shared_ptr<device_context> context,
        const shader_stages_specifications& specifications,
        const viewport_info& viewport_info) noexcept;

    ~pipeline() noexcept;

private:
    std::shared_ptr<device_context> context_;
    underlying_type underlying_ = nullptr;

    VkPipelineLayout layout_ = nullptr;
};
} // namespace zoo::render
