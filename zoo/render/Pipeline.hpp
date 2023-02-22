#pragma once
#include "device_context.hpp"
#include "fwd.hpp"
#include "renderpass.hpp"
#include "stdx/span.hpp"

namespace zoo::render {

class Shader {
public:
    using underlying_type = VkShaderModule;

    operator underlying_type() const noexcept { return get(); }
    underlying_type get() const noexcept { return module_; }

    std::string_view entry_point() const noexcept { return entry_point_; }

    void reset() noexcept;

    Shader(ref<DeviceContext> context, stdx::span<uint32_t> code,
        std::string_view entry_point) noexcept;

    Shader() noexcept;
    ~Shader() noexcept;

    Shader(const Shader& other) = delete;
    Shader& operator=(const Shader& other) = delete;

    Shader(Shader&& other) noexcept;
    Shader& operator=(Shader&& other) noexcept;

private:
    std::shared_ptr<DeviceContext> context_;
    underlying_type module_ = nullptr;
    std::string entry_point_;
};

enum class ShaderType {
    f32, // float
    vec2,
    vec3,
    vec4,
    ivec2,
    uvec4,
    f64 // double
};

// can these be automated?
struct VertexBufferDescription {
    ShaderType type;
    uint32_t offset;
};

struct VertexInputDescription {
    uint32_t location;
    uint32_t stride;
    stdx::span<VertexBufferDescription> buffer_description;
    VkVertexInputRate input_rate = {VK_VERTEX_INPUT_RATE_VERTEX};
};

struct ShaderStagesSpecification {
    const Shader& vertex;
    const Shader& fragment;
    // needs to be in order of vertex buffer supplied
    stdx::span<VertexInputDescription> description = {};
};

class Pipeline {
public:
    using underlying_type = VkPipeline;

    Pipeline(std::shared_ptr<DeviceContext> context,
        const ShaderStagesSpecification& specifications,
        const ViewportInfo& viewport_info,
        const Renderpass& renderpass) noexcept;

    ~Pipeline() noexcept;

    operator underlying_type() const { return get(); }
    underlying_type get() const { return underlying_; }

private:
    std::shared_ptr<DeviceContext> context_;
    underlying_type underlying_ = nullptr;

    VkPipelineLayout layout_ = nullptr;
};
} // namespace zoo::render