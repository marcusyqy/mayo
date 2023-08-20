#pragma once
#include "Device_Context.hpp"
#include "Render_Pass.hpp"
#include "core/fwd.hpp"
#include "fwd.hpp"
#include "stdx/span.hpp"

namespace zoo::render {

class Shader {
public:
    using underlying_type = VkShaderModule;

    operator underlying_type() const noexcept { return get(); }
    underlying_type get() const noexcept { return module_; }

    std::string_view entry_point() const noexcept { return entry_point_; }

    void reset() noexcept;

    Shader(Device_Context& context, stdx::span<uint32_t> code, std::string_view entry_point) noexcept;

    Shader() noexcept;
    ~Shader() noexcept;

    Shader(const Shader& other)            = delete;
    Shader& operator=(const Shader& other) = delete;

    Shader(Shader&& other) noexcept;
    Shader& operator=(Shader&& other) noexcept;

private:
    Device_Context* context_;
    underlying_type module_ = nullptr;
    std::string entry_point_;
};

enum class ShaderType {
    f32, // float
    vec2,
    vec3,
    vec4,
    vec2_unorm,
    vec3_unorm,
    vec4_unorm,
    ivec2,
    uvec4,
    f64 // double
};

enum class ShaderStage { vertex, fragment, geometry };

// can these be automated?
struct VertexBufferDescription {
    u32 location;
    ShaderType type;
    u32 offset;
};

struct VertexInputDescription {
    u32 stride;
    stdx::span<VertexBufferDescription> buffer_description;
    VkVertexInputRate input_rate = { VK_VERTEX_INPUT_RATE_VERTEX };
};

struct ShaderStagesSpecification {
    const Shader& vertex;
    const Shader& fragment;
    // needs to be in order of vertex buffer supplied
    stdx::span<VertexInputDescription> description = {};
};

using PushConstant = VkPushConstantRange;

// NOTE:
// this does not relate to pipeline at all. all the pipeline needs is the layout
// and not the information. only the layout/pool needs this description and we
// should abstract that to the pool or the overarching object.
struct BindingDescriptor {
    VkDescriptorType type;
    u32 count;
    VkShaderStageFlags stage;

    u32 set = 0;
};

struct PipelineCreateInfo {
    bool enable_cull = true; // should we allow choosing of front/back?
};

class Pipeline {
public:
    using underlying_type = VkPipeline;

    Pipeline(
        Device_Context& context,
        const ShaderStagesSpecification& specifications,
        const Render_Pass& renderpass,
        stdx::span<BindingDescriptor> binding_descriptors,
        stdx::span<PushConstant> push_constants,
        const PipelineCreateInfo& create_info = {}) noexcept;

    Pipeline() noexcept                  = default;
    Pipeline(const Pipeline&)            = delete;
    Pipeline& operator=(const Pipeline&) = delete;

    Pipeline(Pipeline&&) noexcept;
    Pipeline& operator=(Pipeline&&) noexcept;

    ~Pipeline() noexcept;

    operator underlying_type() const { return get(); }
    underlying_type get() const { return underlying_; }

    VkPipelineLayout layout() const { return layout_; }

    friend class Descriptor_Pool;

private:
    Device_Context* context_     = nullptr;
    underlying_type underlying_ = nullptr;

    VkPipelineLayout layout_ = nullptr;

    constexpr static u32 MAX_DESCRIPTORS = 5;
    VkDescriptorSetLayout set_layout_[MAX_DESCRIPTORS]{ nullptr };
    u32 set_layout_count_{};
};

} // namespace zoo::render
