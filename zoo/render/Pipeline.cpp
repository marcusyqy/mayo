
#include "Pipeline.hpp"
#include "core/fwd.hpp"
#include "stdx/defer.hpp"

namespace zoo::render {

namespace {

template <ShaderType t>
struct Converter;

template <>
struct Converter<ShaderType::f32> {
    static constexpr VkFormat value = VK_FORMAT_R32_SFLOAT;
};

template <>
struct Converter<ShaderType::vec2> {
    static constexpr VkFormat value = VK_FORMAT_R32G32_SFLOAT;
};

template <>
struct Converter<ShaderType::vec3> {
    static constexpr VkFormat value = VK_FORMAT_R32G32B32_SFLOAT;
};

template <>
struct Converter<ShaderType::vec4> {
    static constexpr VkFormat value = VK_FORMAT_R32G32B32A32_SFLOAT;
};

template <>
struct Converter<ShaderType::ivec2> {
    static constexpr VkFormat value = VK_FORMAT_R32G32_SINT;
};

template <>
struct Converter<ShaderType::uvec4> {
    static constexpr VkFormat value = VK_FORMAT_R32G32B32A32_UINT;
};

template <>
struct Converter<ShaderType::f64> {
    static constexpr VkFormat value = VK_FORMAT_R64_SFLOAT;
};

VkFormat convert_to_shader_stage(ShaderType t) {
    switch (t) {
        case ShaderType::f32: return Converter<ShaderType::f32>::value;
        case ShaderType::vec2: return Converter<ShaderType::vec2>::value;
        case ShaderType::vec3: return Converter<ShaderType::vec3>::value;
        case ShaderType::vec4: return Converter<ShaderType::vec4>::value;
        case ShaderType::ivec2: return Converter<ShaderType::ivec2>::value;
        case ShaderType::uvec4: return Converter<ShaderType::uvec4>::value;
        case ShaderType::f64: return Converter<ShaderType::f64>::value;
    }

    return VK_FORMAT_UNDEFINED;
}

} // namespace

void Shader::reset() noexcept {
    if (module_ != nullptr && context_ != nullptr) vkDestroyShaderModule(*context_, module_, nullptr);

    module_ = nullptr;
}

Shader::Shader(DeviceContext& context, stdx::span<uint32_t> code, std::string_view entry_point) noexcept :
    context_(&context), module_(nullptr), entry_point_(entry_point) {
    VkShaderModuleCreateInfo create_info{};
    create_info.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    create_info.codeSize = sizeof(uint32_t) * code.size();
    create_info.pCode    = code.data();
    // TODO : write allocator
    VK_EXPECT_SUCCESS(vkCreateShaderModule(*context_, &create_info, nullptr, &module_));
}

Shader::Shader() noexcept : context_(nullptr), module_(nullptr), entry_point_() {}

Shader::Shader(Shader&& other) noexcept :
    context_(std::move(other.context_)), module_(std::move(other.module_)),
    entry_point_(std::move(other.entry_point_)) {
    other.context_ = nullptr;
    other.module_  = nullptr;
    other.entry_point_.clear();
}

Shader& Shader::operator=(Shader&& other) noexcept {
    context_       = std::move(other.context_);
    module_        = std::move(other.module_);
    entry_point_   = std::move(other.entry_point_);
    other.context_ = nullptr;
    other.module_  = nullptr;
    other.entry_point_.clear();

    return *this;
}

Shader::~Shader() noexcept { reset(); }

Pipeline::Pipeline(
    DeviceContext& context,
    const ShaderStagesSpecification& specifications,
    const ViewportInfo& viewport_info,
    const RenderPass& renderpass,
    stdx::span<BindingDescriptor> binding_descriptors,
    stdx::span<PushConstant> push_constants) noexcept :
    context_(&context) {

    enum : uint32_t { vertex_stage = 0, fragment_stage = 1, shader_stages = 2 };

    VkPipelineShaderStageCreateInfo shaders_create_info[shader_stages]{};
    {
        VkPipelineShaderStageCreateInfo& vertex_create_info{ shaders_create_info[vertex_stage] };
        vertex_create_info.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertex_create_info.stage  = VK_SHADER_STAGE_VERTEX_BIT;
        vertex_create_info.module = specifications.vertex;
        vertex_create_info.pName  = specifications.vertex.entry_point().data();
    }
    {
        VkPipelineShaderStageCreateInfo& fragment_create_info{ shaders_create_info[fragment_stage] };
        fragment_create_info.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragment_create_info.stage  = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragment_create_info.module = specifications.fragment;
        fragment_create_info.pName  = specifications.fragment.entry_point().data();
    }

    // for reusing
    {
        VkDescriptorSetLayoutBinding* descriptor_set_layouts =
            new VkDescriptorSetLayoutBinding[binding_descriptors.size()];
        STDX_DEFER({ delete[] descriptor_set_layouts; });
        for (set_layout_count_ = 0; set_layout_count_ < MAX_DESCRIPTORS; ++set_layout_count_) {
            // reset.
            auto idx         = set_layout_count_;
            set_layout_[idx] = nullptr;

            u32 i = 0;
            for (const auto& bd : binding_descriptors) {
                if (bd.set == idx) {
                    descriptor_set_layouts[i] = VkDescriptorSetLayoutBinding{
                        .binding         = i,
                        .descriptorType  = bd.type,
                        .descriptorCount = bd.count,
                        .stageFlags      = bd.stage,
                    };
                    ++i;
                }
            }

            if (i == 0) {
                break;
            }

            const auto descriptor_set_count                 = i;
            VkDescriptorSetLayoutCreateInfo set_create_info = { .sType =
                                                                    VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
                                                                .pNext        = nullptr,
                                                                .flags        = 0,
                                                                .bindingCount = descriptor_set_count,
                                                                .pBindings    = descriptor_set_layouts };

            VK_EXPECT_SUCCESS(vkCreateDescriptorSetLayout(context, &set_create_info, nullptr, set_layout_ + idx));
        }
    }

    VkDynamicState dynamic_states_array[]{ VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

    VkPipelineDynamicStateCreateInfo dynamic_state{};
    dynamic_state.sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamic_state.dynamicStateCount = static_cast<uint32_t>(std::size(dynamic_states_array));
    dynamic_state.pDynamicStates    = +dynamic_states_array;

    const auto& viewport = viewport_info.viewport;
    const auto& scissor  = viewport_info.scissor;

    VkPipelineViewportStateCreateInfo viewport_state_create_info{};
    viewport_state_create_info.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_state_create_info.viewportCount = 1;
    viewport_state_create_info.pViewports    = &viewport;
    viewport_state_create_info.scissorCount  = 1;
    viewport_state_create_info.pScissors     = &scissor;

    // this should not be a bottle neck and if it becomes it then maybe we can
    // just try to guess how many attributes we will max use and use an array
    std::vector<VkVertexInputAttributeDescription> vk_vertex_input_attr_desc{};
    std::vector<VkVertexInputBindingDescription> vk_vertex_input_bind_desc{};
    const auto& vertex_description = specifications.description;

    uint32_t binding{};
    for (const auto& desc : vertex_description) {
        for (const auto& buf_desc : desc.buffer_description) {
            VkVertexInputAttributeDescription vk_vertex_input_attr;
            vk_vertex_input_attr.binding  = binding;
            vk_vertex_input_attr.location = buf_desc.location;
            vk_vertex_input_attr.format   = convert_to_shader_stage(buf_desc.type);
            vk_vertex_input_attr.offset   = buf_desc.offset;
            vk_vertex_input_attr_desc.push_back(vk_vertex_input_attr);
        }
        VkVertexInputBindingDescription vk_vertex_input_bind;
        vk_vertex_input_bind.binding   = binding;
        vk_vertex_input_bind.inputRate = desc.input_rate;
        vk_vertex_input_bind.stride    = desc.stride;
        vk_vertex_input_bind_desc.push_back(vk_vertex_input_bind);
        ++binding;
    }

    VkPipelineVertexInputStateCreateInfo vertex_input_state_create_info{};
    vertex_input_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input_state_create_info.vertexBindingDescriptionCount =
        static_cast<u32>(std::size(vk_vertex_input_bind_desc));
    vertex_input_state_create_info.pVertexBindingDescriptions = vk_vertex_input_bind_desc.data();
    vertex_input_state_create_info.vertexAttributeDescriptionCount =
        static_cast<u32>(std::size(vk_vertex_input_attr_desc));
    vertex_input_state_create_info.pVertexAttributeDescriptions = vk_vertex_input_attr_desc.data();

    // INPUT ASSEMBLY
    VkPipelineInputAssemblyStateCreateInfo input_assembly_create_info{};
    input_assembly_create_info.sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly_create_info.topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    input_assembly_create_info.primitiveRestartEnable = VK_FALSE;

    // RASTERIZATION
    VkPipelineRasterizationStateCreateInfo rasterizer_create_info{};
    rasterizer_create_info.sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer_create_info.depthClampEnable        = VK_FALSE;
    rasterizer_create_info.depthBiasEnable         = VK_FALSE;
    rasterizer_create_info.depthBiasConstantFactor = 0.0f; // Optional
    rasterizer_create_info.depthBiasClamp          = 0.0f; // Optional
    rasterizer_create_info.depthBiasSlopeFactor    = 0.0f; // Optional
    rasterizer_create_info.cullMode                = VK_CULL_MODE_BACK_BIT;
    rasterizer_create_info.frontFace               = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer_create_info.lineWidth               = 1.0f;
    rasterizer_create_info.polygonMode             = VK_POLYGON_MODE_FILL;
    rasterizer_create_info.rasterizerDiscardEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampling_create_info{};
    multisampling_create_info.sType                 = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling_create_info.sampleShadingEnable   = VK_FALSE;
    multisampling_create_info.rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT;
    multisampling_create_info.minSampleShading      = 1.0f;     // Optional
    multisampling_create_info.pSampleMask           = nullptr;  // Optional
    multisampling_create_info.alphaToCoverageEnable = VK_FALSE; // Optional
    multisampling_create_info.alphaToOneEnable      = VK_FALSE; // Optional

    VkPipelineColorBlendAttachmentState color_blend_attachment_state{};
    color_blend_attachment_state.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    color_blend_attachment_state.blendEnable         = VK_FALSE;
    color_blend_attachment_state.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;  // Optional
    color_blend_attachment_state.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    color_blend_attachment_state.colorBlendOp        = VK_BLEND_OP_ADD;      // Optional
    color_blend_attachment_state.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;  // Optional
    color_blend_attachment_state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    color_blend_attachment_state.alphaBlendOp        = VK_BLEND_OP_ADD;      // Optional

    VkPipelineColorBlendStateCreateInfo color_blend_state_create_info{};
    color_blend_state_create_info.sType             = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blend_state_create_info.logicOpEnable     = VK_FALSE;
    color_blend_state_create_info.logicOp           = VK_LOGIC_OP_COPY; // Optional
    color_blend_state_create_info.attachmentCount   = 1;
    color_blend_state_create_info.pAttachments      = &color_blend_attachment_state;
    color_blend_state_create_info.blendConstants[0] = 0.0f; // Optional
    color_blend_state_create_info.blendConstants[1] = 0.0f; // Optional
    color_blend_state_create_info.blendConstants[2] = 0.0f; // Optional
    color_blend_state_create_info.blendConstants[3] = 0.0f; // Optional
                                                            //
    VkPipelineLayoutCreateInfo pipeline_layout_create_info{};
    pipeline_layout_create_info.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_create_info.setLayoutCount         = set_layout_count_;
    pipeline_layout_create_info.pSetLayouts            = +set_layout_;
    pipeline_layout_create_info.pushConstantRangeCount = static_cast<u32>(push_constants.size());
    pipeline_layout_create_info.pPushConstantRanges    = push_constants.data();

    VK_EXPECT_SUCCESS(
        vkCreatePipelineLayout(*context_, &pipeline_layout_create_info, nullptr, &layout_),
        [](VkResult /* result */) {
            ZOO_LOG_ERROR("Pipeline layout creation failed, maybe we should "
                          "assert here?");
        });
    VkPipelineDepthStencilStateCreateInfo depth_stencil_state_info = {};
    depth_stencil_state_info.sType                 = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depth_stencil_state_info.pNext                 = nullptr;
    depth_stencil_state_info.depthTestEnable       = VK_TRUE;
    depth_stencil_state_info.depthWriteEnable      = VK_TRUE;
    depth_stencil_state_info.depthCompareOp        = VK_COMPARE_OP_LESS_OR_EQUAL;
    depth_stencil_state_info.depthBoundsTestEnable = VK_FALSE;
    depth_stencil_state_info.minDepthBounds        = 0.0f; // Optional
    depth_stencil_state_info.maxDepthBounds        = 1.0f; // Optional
    depth_stencil_state_info.stencilTestEnable     = VK_FALSE;

    VkGraphicsPipelineCreateInfo graphics_pipeline_create_info{};
    graphics_pipeline_create_info.sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    graphics_pipeline_create_info.stageCount          = 2;
    graphics_pipeline_create_info.pStages             = +shaders_create_info;
    graphics_pipeline_create_info.pVertexInputState   = &vertex_input_state_create_info;
    graphics_pipeline_create_info.pInputAssemblyState = &input_assembly_create_info;
    graphics_pipeline_create_info.pViewportState      = &viewport_state_create_info;
    graphics_pipeline_create_info.pRasterizationState = &rasterizer_create_info;
    graphics_pipeline_create_info.pMultisampleState   = &multisampling_create_info;
    graphics_pipeline_create_info.pDepthStencilState  = &depth_stencil_state_info; // Optional
    graphics_pipeline_create_info.pColorBlendState    = &color_blend_state_create_info;
    graphics_pipeline_create_info.pDynamicState       = &dynamic_state;
    graphics_pipeline_create_info.layout              = layout_;
    graphics_pipeline_create_info.renderPass          = renderpass;
    graphics_pipeline_create_info.subpass             = 0;
    graphics_pipeline_create_info.basePipelineHandle  = VK_NULL_HANDLE; // Optional
    graphics_pipeline_create_info.basePipelineIndex   = -1;             // Optional

    VK_EXPECT_SUCCESS(
        vkCreateGraphicsPipelines(*context_, nullptr, 1, &graphics_pipeline_create_info, nullptr, &underlying_));
}

Pipeline::Pipeline(Pipeline&& o) noexcept { *this = std::move(o); }

Pipeline& Pipeline::operator=(Pipeline&& o) noexcept {
    this->~Pipeline();

    o.context_        = o.context_;
    o.underlying_     = o.underlying_;
    layout_           = o.layout_;
    set_layout_count_ = o.set_layout_count_;

    memcpy(set_layout_, o.set_layout_, sizeof(o.set_layout_));

    o.context_          = nullptr;
    o.underlying_       = nullptr;
    o.layout_           = nullptr;
    o.set_layout_count_ = {};

    memset(o.set_layout_, 0, sizeof(o.set_layout_));

    return *this;
}

Pipeline::~Pipeline() noexcept {
    if (context_) {
        vkDestroyPipelineLayout(*context_, layout_, nullptr);

        for (u32 i = 0; i < set_layout_count_; ++i) {
            vkDestroyDescriptorSetLayout(*context_, set_layout_[i], nullptr);
        }

        vkDestroyPipeline(*context_, underlying_, nullptr);
    }
}

} // namespace zoo::render
