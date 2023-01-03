
#include "pipeline.hpp"
namespace zoo::render {

void shader::reset() noexcept {
    if (module_ != nullptr && context_)
        vkDestroyShaderModule(*context_, module_, nullptr);

    module_ = nullptr;
}

shader::shader(std::shared_ptr<device_context> context, stdx::span<char> code,
    std::string_view entry_point) noexcept :
    context_(context),
    module_(nullptr), entry_point_(entry_point) {
    VkShaderModuleCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    create_info.codeSize = code.size();
    create_info.pCode = reinterpret_cast<const uint32_t*>(code.data());
    // TODO : write allocator
    VK_EXPECT_SUCCESS(
        vkCreateShaderModule(*context_, &create_info, nullptr, &module_));
}

shader::shader() noexcept :
    context_(nullptr), module_(nullptr), entry_point_() {}

shader::shader(shader&& other) noexcept :
    context_(std::move(other.context_)), module_(std::move(other.module_)),
    entry_point_(std::move(other.entry_point_)) {
    other.context_ = nullptr;
    other.module_ = nullptr;
    other.entry_point_.clear();
}

shader& shader::operator=(shader&& other) noexcept {
    context_ = std::move(other.context_);
    module_ = std::move(other.module_);
    entry_point_ = std::move(other.entry_point_);
    other.context_ = nullptr;
    other.module_ = nullptr;
    other.entry_point_.clear();
    return *this;
}

shader::~shader() noexcept { reset(); }

// TODO: do some cleanup in this area.
pipeline::pipeline(std::shared_ptr<device_context> context,
    const shader_stages_specifications& specifications,
    const viewport_info& viewport_info) noexcept :
    context_(context) {

    constexpr uint32_t vertex_stage = 0;
    constexpr uint32_t fragment_stage = 1;
    constexpr uint32_t shader_stages = 2;

    VkPipelineShaderStageCreateInfo shaders_create_info[shader_stages];
    {
        VkPipelineShaderStageCreateInfo& vertex_create_info{
            shaders_create_info[vertex_stage]};
        vertex_create_info.sType =
            VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertex_create_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertex_create_info.module = specifications.vertex;
        vertex_create_info.pName = specifications.vertex.entry_point().data();
    }
    {
        VkPipelineShaderStageCreateInfo& fragment_create_info{
            shaders_create_info[fragment_stage]};
        fragment_create_info.sType =
            VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragment_create_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
        fragment_create_info.module = specifications.fragment;
        fragment_create_info.pName =
            specifications.fragment.entry_point().data();
    }

    VkDynamicState dynamic_states_array[]{
        VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

    VkPipelineDynamicStateCreateInfo dynamic_state{};
    dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamic_state.dynamicStateCount =
        static_cast<uint32_t>(std::size(dynamic_states_array));
    dynamic_state.pDynamicStates = +dynamic_states_array;

    const auto& viewport = viewport_info.viewport;
    const auto& scissor = viewport_info.scissor;

    VkPipelineViewportStateCreateInfo viewport_state_create_info{};
    viewport_state_create_info.sType =
        VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_state_create_info.viewportCount = 1;
    viewport_state_create_info.pViewports = &viewport;
    viewport_state_create_info.scissorCount = 1;
    viewport_state_create_info.pScissors = &scissor;

    VkPipelineVertexInputStateCreateInfo vertex_input_state_create_info{};
    vertex_input_state_create_info.sType =
        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input_state_create_info.vertexBindingDescriptionCount = 0;
    vertex_input_state_create_info.pVertexBindingDescriptions =
        nullptr; // Optional
    vertex_input_state_create_info.vertexAttributeDescriptionCount = 0;
    vertex_input_state_create_info.pVertexAttributeDescriptions =
        nullptr; // Optional

    VkPipelineInputAssemblyStateCreateInfo input_assembly_create_info{};
    input_assembly_create_info.sType =
        VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly_create_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    input_assembly_create_info.primitiveRestartEnable = VK_FALSE;

    VkPipelineRasterizationStateCreateInfo rasterizer_create_info{};
    rasterizer_create_info.sType =
        VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer_create_info.depthClampEnable = VK_FALSE;
    rasterizer_create_info.depthBiasEnable = VK_FALSE;
    rasterizer_create_info.depthBiasConstantFactor = 0.0f; // Optional
    rasterizer_create_info.depthBiasClamp = 0.0f;          // Optional
    rasterizer_create_info.depthBiasSlopeFactor = 0.0f;    // Optional
    rasterizer_create_info.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer_create_info.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer_create_info.lineWidth = 1.0f;
    rasterizer_create_info.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer_create_info.rasterizerDiscardEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampling_create_info{};
    multisampling_create_info.sType =
        VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling_create_info.sampleShadingEnable = VK_FALSE;
    multisampling_create_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling_create_info.minSampleShading = 1.0f;          // Optional
    multisampling_create_info.pSampleMask = nullptr;            // Optional
    multisampling_create_info.alphaToCoverageEnable = VK_FALSE; // Optional
    multisampling_create_info.alphaToOneEnable = VK_FALSE;      // Optional
    //
    VkPipelineColorBlendAttachmentState color_blend_attachment_state{};
    color_blend_attachment_state.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    color_blend_attachment_state.blendEnable = VK_FALSE;
    color_blend_attachment_state.srcColorBlendFactor =
        VK_BLEND_FACTOR_ONE; // Optional
    color_blend_attachment_state.dstColorBlendFactor =
        VK_BLEND_FACTOR_ZERO;                                    // Optional
    color_blend_attachment_state.colorBlendOp = VK_BLEND_OP_ADD; // Optional
    color_blend_attachment_state.srcAlphaBlendFactor =
        VK_BLEND_FACTOR_ONE; // Optional
    color_blend_attachment_state.dstAlphaBlendFactor =
        VK_BLEND_FACTOR_ZERO;                                    // Optional
    color_blend_attachment_state.alphaBlendOp = VK_BLEND_OP_ADD; // Optional/

    VkPipelineColorBlendStateCreateInfo color_blend_state_create_info{};
    color_blend_state_create_info.sType =
        VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blend_state_create_info.logicOpEnable = VK_FALSE;
    color_blend_state_create_info.logicOp = VK_LOGIC_OP_COPY; // Optional
    color_blend_state_create_info.attachmentCount = 1;
    color_blend_state_create_info.pAttachments = &color_blend_attachment_state;
    color_blend_state_create_info.blendConstants[0] = 0.0f; // Optional
    color_blend_state_create_info.blendConstants[1] = 0.0f; // Optional
    color_blend_state_create_info.blendConstants[2] = 0.0f; // Optional
    color_blend_state_create_info.blendConstants[3] = 0.0f; // Optional
                                                            //
    VkPipelineLayoutCreateInfo pipeline_layout_create_info{};
    pipeline_layout_create_info.sType =
        VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_create_info.setLayoutCount = 0;            // Optional
    pipeline_layout_create_info.pSetLayouts = nullptr;         // Optional
    pipeline_layout_create_info.pushConstantRangeCount = 0;    // Optional
    pipeline_layout_create_info.pPushConstantRanges = nullptr; // Optional

    VK_EXPECT_SUCCESS(vkCreatePipelineLayout(*context_,
                          &pipeline_layout_create_info, nullptr, &layout_),
        [](VkResult /* result */) {
            ZOO_LOG_ERROR("Pipeline layout creation failed, maybe we should "
                          "assert here?");
        })
}

pipeline::~pipeline() noexcept {
    if (context_)
        vkDestroyPipelineLayout(*context_, layout_, nullptr);
}

} // namespace zoo::render
