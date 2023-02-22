#pragma once
#include "device_context.hpp"
#include "fwd.hpp"
#include "renderpass.hpp"
#include "stdx/span.hpp"

namespace zoo::render {

class shader {
public:
    using underlying_type = VkShaderModule;

    operator underlying_type() const noexcept { return get(); }
    underlying_type get() const noexcept { return module_; }

    std::string_view entry_point() const noexcept { return entry_point_; }

    void reset() noexcept;

    shader(ref<device_context> context, stdx::span<uint32_t> code,
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

enum class shader_type {
    f32, // float
    vec2,
    vec3,
    vec4,
    ivec2,
    uvec4,
    f64 // double
};

// can these be automated?
struct vertex_buffer_description {
    shader_type type;
    uint32_t offset;
};

struct vertex_input_description {
    uint32_t location;
    uint32_t stride;
    stdx::span<vertex_buffer_description> buffer_description;
    VkVertexInputRate input_rate = {VK_VERTEX_INPUT_RATE_VERTEX};
};

struct shader_stages_specifications {
    const shader& vertex;
    const shader& fragment;
    // needs to be in order of vertex buffer supplied
    stdx::span<vertex_input_description> description = {};
};

class pipeline {
public:
    using underlying_type = VkPipeline;

    pipeline(std::shared_ptr<device_context> context,
        const shader_stages_specifications& specifications,
        const viewport_info& viewport_info,
        const renderpass& renderpass) noexcept;

    ~pipeline() noexcept;

    operator underlying_type() const { return get(); }
    underlying_type get() const { return underlying_; }

private:
    std::shared_ptr<device_context> context_;
    underlying_type underlying_ = nullptr;

    VkPipelineLayout layout_ = nullptr;
};
} // namespace zoo::render
