#include "scene.hpp"

#include "core/log.hpp"
#include "core/utils.hpp"
#include "core/window.hpp"

#include "render/descriptor_pool.hpp"
#include "render/engine.hpp"
#include "render/framebuffer.hpp"
#include "render/pipeline.hpp"
#include "render/resources/buffer.hpp"
#include "render/resources/mesh.hpp"
#include "render/scene/command_buffer.hpp"
#include "render/scene/upload_context.hpp"
#include "render/swapchain.hpp"
#include "render/sync/fence.hpp"

#include "stdx/expected.hpp"

#include "imgui/layer.hpp"
#include "tools/shader_compiler.hpp"

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#include <array>
#include <fstream>
#include <memory>
#include <string_view>

#include <stb_image.h>

namespace zoo {

namespace {

struct Scene_Data {
    glm::vec4 fog_color;          // w is for exponent
    glm::vec4 fog_distance;       // x for min, y for max, zw unused
    glm::vec4 ambient_color;
    glm::vec4 sunlight_direction; // w for sunpower
    glm::vec4 sunlight_color;
};

struct Uniform_Buffer_Data {
    glm::mat4 view;
    glm::mat4 proj;
    glm::mat4 viewproj;
};

struct Object_Data {
    glm::mat4 model_mat;
};

struct Push_Constant_Data {
    glm::vec4 data;
    glm::mat4 render_matrix;
};

using Shader_Bytes = std::vector<u32>;

struct Shaders {
    Shader_Bytes vertex;
    Shader_Bytes fragment;
};

render::PushConstant push_constant{
    .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
    .offset     = 0,
    .size       = sizeof(Push_Constant_Data),
};

constexpr VkFormat COLOR_IMAGE_FORMAT = VK_FORMAT_R8G8B8A8_SRGB;
constexpr VkFormat DEPTH_FORMAT       = VK_FORMAT_D32_SFLOAT;

render::resources::Texture create_render_buffer(render::Device_Context& context, u32 x, u32 y) noexcept {
    return render::resources::Texture::start_build("RT-ImguiFrameBuffer")
        .format(COLOR_IMAGE_FORMAT)
        .usage(VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
        .extent({ x, y, 1 })
        .allocation_type(VMA_MEMORY_USAGE_GPU_ONLY)
        .allocation_required_flags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
        .build(context.allocator());
}

render::resources::Texture create_depth_buffer(render::Device_Context& context, u32 x, u32 y) noexcept {
    return render::resources::Texture::start_build("Depth-ImguiFrameBuffer")
        .format(DEPTH_FORMAT)
        .usage(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
        .extent({ x, y, 1 })
        .allocation_type(VMA_MEMORY_USAGE_GPU_ONLY)
        .allocation_required_flags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
        .build(context.allocator());
}

Shaders read_shaders() noexcept {

    tools::Shader_Compiler compiler;
    auto vertex_bytes = core::read_file("static/shaders/Test.vert");
    ZOO_ASSERT(vertex_bytes, "vertex shader must have value!");
    auto fragment_bytes = core::read_file("static/shaders/Test.frag");
    ZOO_ASSERT(fragment_bytes, "fragment shader must have value!");

    tools::Shader_Work vertex_work{ shaderc_vertex_shader, "Test.vert", *vertex_bytes };
    tools::Shader_Work fragment_work{ shaderc_fragment_shader, "Test.frag", *fragment_bytes };

    auto vertex_spirv   = compiler.compile(vertex_work);
    auto fragment_spirv = compiler.compile(fragment_work);

    if (!vertex_spirv) {
        spdlog::error("Vertex has error : {}", vertex_spirv.error().what());
    }

    if (!fragment_spirv) {
        spdlog::error("Fragment has error : {}", fragment_spirv.error().what());
    }

    return { .vertex = std::move(*vertex_spirv), .fragment = std::move(*fragment_spirv) };
}

render::resources::Texture load_image_from_file(
    render::Device_Context& context,
    render::scene::Upload_Context& upload_context,
    std::string_view file_name) {

    struct Texture_Load_Details {
        s32 width;
        s32 height;
        s32 channels;
    } tex_details;

    struct Stbi_Image_Free {
        void operator()(stbi_uc* data) const noexcept { stbi_image_free(data); }
    };

    std::unique_ptr<stbi_uc, Stbi_Image_Free> pixels{
        stbi_load(file_name.data(), &tex_details.width, &tex_details.height, &tex_details.channels, STBI_rgb_alpha)
    };

    if (!pixels) return {};

    void* pixel_ptr   = reinterpret_cast<void*>(pixels.get());
    size_t image_size = tex_details.width * tex_details.height * 4;

    render::resources::Buffer scratch_buffer =
        render::resources::Buffer::start_build(fmt::format("ScratchBuffer for texture : {}", file_name), image_size)
            .usage(VK_BUFFER_USAGE_TRANSFER_SRC_BIT)
            .allocation_type(VMA_MEMORY_USAGE_AUTO_PREFER_HOST)
            .allocation_flag(VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT)
            .build(context.allocator());

    scratch_buffer.map(
        [&pixel_ptr, image_size](void* data) noexcept { memcpy(data, pixel_ptr, static_cast<size_t>(image_size)); });

    VkFormat image_format = VK_FORMAT_R8G8B8A8_SRGB;
    auto texture          = render::resources::Texture::start_build(file_name)
                       .format(image_format)
                       .extent(VkExtent3D{
                           .width  = static_cast<u32>(tex_details.width),
                           .height = static_cast<u32>(tex_details.height),
                           .depth  = 1,
                       })
                       .usage(VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT)
                       .allocation_type(VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE)
                       .allocation_required_flags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
                       .build(context.allocator());

    upload_context.copy(scratch_buffer, texture);
    upload_context.transition_to_shader_read(texture);
    upload_context.cache(std::move(scratch_buffer));
    return texture;
}

// TODO: move this to a utils :: namespace or something.
template <typename T, std::enable_if_t<std::is_integral_v<T>, bool> = true>
T is_power_of_two(T v) noexcept {
    return (((v) != 0) && (((v) & ((v)-1)) == 0));
}

size_t pad_uniform_buffer_size(const render::Device_Context& context, size_t original_size) {
    size_t min_ubo_alignment = context.physical().limits().minUniformBufferOffsetAlignment;
    size_t aligned_size      = original_size;

    if (min_ubo_alignment > 0) aligned_size = (aligned_size + min_ubo_alignment - 1) & ~(min_ubo_alignment - 1);

    return aligned_size;
}

} // namespace

Imgui_Scene::Imgui_Scene(render::Engine& engine, s32 width, s32 height) noexcept :
    engine_(engine), width_(width), height_(height) {
    init();
}

void Imgui_Scene::init() noexcept {
    auto& context                       = engine_.context();
    auto [vertex_bytes, fragment_bytes] = read_shaders();
    render::Shader vertex_shader{ context, vertex_bytes, "main" };
    render::Shader fragment_shader{ context, fragment_bytes, "main" };

    render::scene::Upload_Context upload_cmd_buffer{ context };

    // upload gpu memory
    mesh_                = { context.allocator(), upload_cmd_buffer, "static/assets", "lost_empire.obj" };
    lost_empire_         = load_image_from_file(context, upload_cmd_buffer, "static/assets/lost_empire-RGBA.png");
    lost_empire_sampler_ = render::resources::TextureSampler::start_build()
                               .mag_filter(VK_FILTER_NEAREST)
                               .min_filter(VK_FILTER_NEAREST)
                               .build(context);
    upload_cmd_buffer.submit();

    render::AttachmentDescription attachments[] = { render::ColorAttachmentDescription(COLOR_IMAGE_FORMAT),
                                                    render::DepthAttachmentDescription() };
    attachments[0].description.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    renderpass_ = { context, attachments };

    auto buffer_description = render::resources::Vertex::describe();
    std::array vertex_description{ render::VertexInputDescription{ sizeof(render::resources::Vertex),
                                                                   buffer_description,
                                                                   VK_VERTEX_INPUT_RATE_VERTEX } };

    render::BindingDescriptor binding_descriptors[] = {
        { .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, .count = 1, .stage = VK_SHADER_STAGE_VERTEX_BIT },
        { .type  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
          .count = 1,
          .stage = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT },
        { .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, .count = 1, .stage = VK_SHADER_STAGE_VERTEX_BIT, .set = 1 },
        { .type  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
          .count = 1,
          .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
          .set   = 2 }
    };

    pipeline_ = { context,
                  render::ShaderStagesSpecification{ vertex_shader, fragment_shader, vertex_description },
                  renderpass_,
                  binding_descriptors,
                  { &push_constant, 1 } };

    descriptor_pool_ = { context };

    scene_data_buffer_ = render::resources::Buffer::start_build(
                             "SceneDataBuffer",
                             MAX_FRAMES * pad_uniform_buffer_size(context, sizeof(Scene_Data)))
                             .usage(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT)
                             .allocation_type(VMA_MEMORY_USAGE_AUTO)
                             .allocation_flag(VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT)
                             .build(context.allocator());

    start_time_ = std::chrono::high_resolution_clock::now();
    upload_cmd_buffer.wait();
}

Imgui_Scene::~Imgui_Scene() noexcept { exit(); }

void Imgui_Scene::exit() noexcept {
    // TODO: we can remove this after we find out how to properly tie
    // resources to each frame.
    engine_.context().wait();
}

void Imgui_Scene::allocate_frame_buffer(const render::Pipeline& pipeline) noexcept {
    auto& context = engine_.context();
    render::resources::BufferView buffer_view{ scene_data_buffer_, 0, sizeof(Scene_Data) };

    for (s32 i = 0; i < MAX_FRAMES; ++i) {
        auto& frame_data = frame_datas_[i];

        const auto uniform_buffer_name = fmt::format("Uniform buffer : {}", i);
        const auto object_buffer_name  = fmt::format("Object buffer : {}", i);
        frame_data.uniform_buffer = render::resources::Buffer::start_build<Uniform_Buffer_Data>(uniform_buffer_name)
                                        .usage(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT)
                                        .allocation_type(VMA_MEMORY_USAGE_CPU_TO_GPU)
                                        .build(context.allocator());

        frame_data.object_storage_buffer = render::resources::Buffer::start_build<Object_Data>(object_buffer_name)
                                               .count(MAX_OBJECTS)
                                               .usage(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT)
                                               .allocation_type(VMA_MEMORY_USAGE_CPU_TO_GPU)
                                               .build(context.allocator());

        frame_data.bindings = descriptor_pool_.allocate(pipeline_);

        // clang-format off
       frame_data.bindings.start_batch()
           .bind(0, frame_data.uniform_buffer,VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
           .bind(1, buffer_view, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC)
           // @EVALUATE : check if this good enough? { 1, 0 } maybe better.
           .bind(1, 0 , frame_data.object_storage_buffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER)
           .bind(2, 0, lost_empire_, lost_empire_sampler_)
           .end_batch();
        // clang-format on
        frame_data.command_buffer  = render::scene::Command_Buffer{ context, render::Operation::graphics };
        frame_data.in_flight_fence = render::sync::Fence{ context, true };

        frame_data.render_buffer  = create_render_buffer(context, width_, height_);
        frame_data.render_sampler = render::resources::TextureSampler::start_build()
                                        .mag_filter(VK_FILTER_NEAREST)
                                        .min_filter(VK_FILTER_NEAREST)
                                        .build(context);
        frame_data.depth_buffer                    = create_depth_buffer(context, width_, height_);
        const render::resources::TextureView* tv[] = { &(frame_data.render_buffer.view()),
                                                       &(frame_data.depth_buffer.view()) };
        frame_data.render_target  = render::Framebuffer{ context, renderpass_, tv, (u32)width_, (u32)height_ };
        frame_data.render_binding = descriptor_pool_.allocate(pipeline);

        frame_data.render_binding.start_batch()
            .bind(0, frame_data.render_buffer, frame_data.render_sampler, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
            .end_batch();

        frame_data.width = width_;
        frame_data.height = height_;
    }
}

const render::Resource_Bindings*
    Imgui_Scene::ensure_frame_buffers_and_update(const render::Pipeline& pipeline, s32 width, s32 height) noexcept {
    
    if (width <= 0) return nullptr;
    if (height <= 0) return nullptr;

    width_ = width;
    height_ = height;

    auto& context = engine_.context();
    bool resized = false;
    auto& frame_data = frame_datas_[index_];
    frame_data.in_flight_fence.wait();
    frame_data.in_flight_fence.reset();

    if (width_ != frame_data.width) {
        resized = true;
        frame_data.width = width_;
    }

    if (height_ != frame_data.height) {
        resized = true;
        frame_data.height = height_;
    }

    if (resized) {
        frame_data.render_buffer                   = create_render_buffer(context, width, height);
        frame_data.depth_buffer                    = create_depth_buffer(context, width, height);
        const render::resources::TextureView* tv[] = { &(frame_data.render_buffer.view()),
                                                        &(frame_data.depth_buffer.view()) };
        frame_data.render_target  = render::Framebuffer{ context, renderpass_, tv, (u32)width, (u32)height };
        frame_data.render_binding = descriptor_pool_.allocate(pipeline);

        frame_data.render_binding.start_batch()
            .bind(0, frame_data.render_buffer, frame_data.render_sampler, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
            .end_batch();
    }
    return &update();
}

const render::Resource_Bindings& Imgui_Scene::update() noexcept {
    Push_Constant_Data push_constant_data{};
    auto& frame_data = frame_datas_[index_];
    auto& context    = engine_.context();

    glm::vec3 cam_pos    = { 0.f, -6.f, -10.f };
    glm::mat4 view       = glm::translate(glm::mat4(1.f), cam_pos);
    glm::mat4 projection = glm::perspective(glm::radians(70.f), 1700.f / 900.f, 0.1f, 200.0f);
    projection[1][1] *= -1;

    frame_data.uniform_buffer.map<Uniform_Buffer_Data>([&](Uniform_Buffer_Data* data) {
        if (data) {
            data->view     = view;
            data->proj     = projection;
            data->viewproj = projection * view;
        }
    });

    auto current_time = std::chrono::high_resolution_clock::now();
    f32 time          = std::chrono::duration<f32, std::chrono::seconds::period>(current_time - start_time_).count();
    f32 var           = time * glm::radians(360.0f);

    u32 offset = static_cast<u32>(index_ * pad_uniform_buffer_size(context, sizeof(Scene_Data)));
    render::resources::BufferView scene_data_buffer_view{ scene_data_buffer_, offset, offset + sizeof(Scene_Data) };
    scene_data_buffer_view.map<Scene_Data>([&](Scene_Data* data) {
        if (data) data->ambient_color = { sin(var), 0, cos(var), 1 };
    });

    glm::mat4 model =
        glm::translate(glm::vec3{ 5, -10, 0 });

    // glm::rotate(glm::mat4{ 1.0f }, time * glm::radians(90.0f), glm::vec3(0, 1, 0));
    push_constant_data.render_matrix = model;
    frame_data.object_storage_buffer.map<Object_Data>([&](Object_Data* data) {
        if (data != nullptr) {
            data[0].model_mat = model;
        }
    });

    auto& command_context = frame_data.command_buffer;

    VkViewport viewport{ .x        = 0.0f,
                         .y        = 0.0f,
                         .width    = (f32)width_,
                         .height   = (f32)height_,
                         .minDepth = 0.0f,
                         .maxDepth = 1.0f };

    VkRect2D scissor{
        .offset = { 0, 0 },
        .extent = { (u32)width_, (u32)height_ },
    };

    command_context.set_viewport(viewport);
    command_context.set_scissor(scissor);

    VkClearValue depth_clear{};
    depth_clear.depthStencil.depth = 1.f;
    VkClearValue clear_color[]     = { { { { 0.1f, 0.1f, 0.1f, 1.0f } } }, depth_clear };
    command_context.begin_renderpass(frame_data.render_target, clear_color);

    command_context.bind_pipeline(pipeline_);
    command_context.push_constants(push_constant, &push_constant_data);
    command_context.bind_resources(frame_data.bindings, { &offset, 1 });

    command_context.bind_mesh(mesh_);
    command_context.draw_indexed(1);

    command_context.end_renderpass();
    command_context.submit(nullptr, nullptr, nullptr, frame_data.in_flight_fence);

    index_ = (index_ + 1) % MAX_FRAMES;
    return frame_data.render_binding;
}

} // namespace zoo
