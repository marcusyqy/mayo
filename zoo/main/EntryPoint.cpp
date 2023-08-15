#include "EntryPoint.hpp"

#include "Utility.hpp"

#include "core/Array.hpp"
#include "core/Log.hpp"
#include "core/platform/Window.hpp"

#include "render/DescriptorPool.hpp"
#include "render/Engine.hpp"
#include "render/Framebuffer.hpp"
#include "render/Pipeline.hpp"
#include "render/resources/Buffer.hpp"
#include "render/resources/Mesh.hpp"
#include "render/scene/CommandBuffer.hpp"
#include "render/scene/UploadContext.hpp"
#include "render/sync/Fence.hpp"

#include "stdx/expected.hpp"

#include "adapters/tools/ShaderCompiler.hpp"
#include "adapters/imgui/Layer.hpp"

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#include <array>
#include <fstream>
#include <memory>
#include <string_view>

#include <stb_image.h>

namespace zoo {

namespace {

struct SceneData {
    glm::vec4 fog_color;    // w is for exponent
    glm::vec4 fog_distance; // x for min, y for max, zw unused
    glm::vec4 ambient_color;
    glm::vec4 sunlight_direction; // w for sunpower
    glm::vec4 sunlight_color;
};

struct FrameData {
    render::resources::Buffer uniform_buffer;
    render::ResourceBindings bindings;
    render::resources::Buffer object_storage_buffer;

    render::resources::Texture depth_buffer;
    render::Framebuffer render_target;

    render::scene::CommandBuffer command_buffer;
    render::sync::Fence in_flight_fence;
};

struct UniformBufferData {
    glm::mat4 view;
    glm::mat4 proj;
    glm::mat4 viewproj;
};

constexpr s32 MAX_FRAMES  = 3;
constexpr s32 MAX_OBJECTS = 10'000;

using FrameDatas = FrameData[MAX_FRAMES];

struct ObjectData {
    glm::mat4 model_mat;
};

struct PushConstantData {
    glm::vec4 data;
    glm::mat4 render_matrix;
};

using Shader_Bytes = std::vector<u32>;

struct Shaders {
    Shader_Bytes vertex;
    Shader_Bytes fragment;
};

render::resources::Texture create_depth_buffer(render::DeviceContext& context, u32 x, u32 y) noexcept {
    static constexpr VkFormat DEPTH_FORMAT_ = VK_FORMAT_D32_SFLOAT;
    return render::resources::Texture::start_build("DepthBufferSwapchain")
        .format(DEPTH_FORMAT_)
        .usage(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
        .extent({ x, y, 1 })
        .allocation_type(VMA_MEMORY_USAGE_GPU_ONLY)
        .allocation_required_flags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
        .build(context.allocator());
}

Shaders read_shaders() noexcept {

    adapters::tools::ShaderCompiler compiler;
    auto vertex_bytes = read_file("static/shaders/Test.vert");
    ZOO_ASSERT(vertex_bytes, "vertex shader must have value!");
    auto fragment_bytes = read_file("static/shaders/Test.frag");
    ZOO_ASSERT(fragment_bytes, "fragment shader must have value!");

    adapters::tools::ShaderWork vertex_work{ shaderc_vertex_shader, "Test.vert", *vertex_bytes };
    adapters::tools::ShaderWork fragment_work{ shaderc_fragment_shader, "Test.frag", *fragment_bytes };

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
    render::DeviceContext& context,
    render::scene::UploadContext& upload_context,
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

    VkFormat image_format = VK_FORMAT_R8G8B8A8_SRGB;

    render::resources::Buffer scratch_buffer =
        render::resources::Buffer::start_build(fmt::format("ScratchBuffer for texture : {}", file_name), image_size)
            .usage(VK_BUFFER_USAGE_TRANSFER_SRC_BIT)
            .allocation_type(VMA_MEMORY_USAGE_AUTO_PREFER_HOST)
            .allocation_flag(VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT)
            .build(context.allocator());

    scratch_buffer.map(
        [&pixel_ptr, image_size](void* data) noexcept { memcpy(data, pixel_ptr, static_cast<size_t>(image_size)); });

    auto texture = render::resources::Texture::start_build(file_name)
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

void* ptr_round_up_align(void* ptr, uintptr_t align) noexcept {
    ZOO_ASSERT(is_power_of_two(align), "align must be a power of two!");
    return (void*)(((uintptr_t)ptr + (align - 1)) & ~(align - 1));
}

void* ptr_round_down_align(void* ptr, uintptr_t align) noexcept {
    ZOO_ASSERT(is_power_of_two(align), "align must be a power of two!");
    return (void*)((uintptr_t)ptr & ~(align - 1));
}

size_t pad_uniform_buffer_size(const render::DeviceContext& context, size_t original_size) {
    size_t min_ubo_alignment = context.physical().limits().minUniformBufferOffsetAlignment;
    size_t aligned_size      = original_size;

    if (min_ubo_alignment > 0) aligned_size = (aligned_size + min_ubo_alignment - 1) & ~(min_ubo_alignment - 1);

    return aligned_size;
}

} // namespace

// application::ExitStatus minecraft_world() noexcept {
//     const application::Info app_context{ { 0, 0, 0 }, "Zoo::Application" };
//     const render::engine::Info render_engine_info{ app_context, true };
//
//     ZOO_LOG_INFO("Starting application");
//
//     render::Engine render_engine{ render_engine_info };
//     auto& context = render_engine.context();
//
//     // TODO: I think we should just merge swapchain and window
//     Window main_window{ render_engine,
//                         window::Traits{ window::Size{ 1280, 960 }, false, "Zoo" },
//                         [](Window& win, input::KeyCode keycode) {
//                             if (keycode.key_ == input::Key::escape && keycode.action_ == input::Action::pressed) {
//                                 win.close();
//                             }
//                         } };
//
//     auto [vertex_bytes, fragment_bytes] = read_shaders();
//
//     render::Shader vertex_shader{ context, vertex_bytes, "main" };
//     render::Shader fragment_shader{ context, fragment_bytes, "main" };
//
//     render::scene::UploadContext upload_cmd_buffer{ context };
//
//     // upload gpu memory
//     render::resources::Mesh mesh{ context.allocator(), upload_cmd_buffer, "static/assets", "lost_empire.obj" };
//     render::resources::Texture lost_empire =
//         load_image_from_file(context, upload_cmd_buffer, "static/assets/lost_empire-RGBA.png");
//     render::resources::TextureSampler lost_empire_sampler = render::resources::TextureSampler::start_build()
//                                                                 .mag_filter(VK_FILTER_NEAREST)
//                                                                 .min_filter(VK_FILTER_NEAREST)
//                                                                 .build(context);
//     upload_cmd_buffer.submit();
//
//     auto& swapchain   = main_window.swapchain();
//     auto image_format = swapchain.format();
//
//     render::AttachmentDescription attachments[] = { render::ColorAttachmentDescription(image_format),
//                                                     render::DepthAttachmentDescription() };
//
//     render::RenderPass renderpass{ context, attachments };
//
//     auto buffer_description = render::resources::Vertex::describe();
//     std::array vertex_description{ render::VertexInputDescription{ sizeof(render::resources::Vertex),
//                                                                    buffer_description,
//                                                                    VK_VERTEX_INPUT_RATE_VERTEX } };
//
//     render::PushConstant push_constant{};
//     push_constant.size       = sizeof(PushConstantData);
//     push_constant.offset     = 0;
//     push_constant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
//
//     render::BindingDescriptor binding_descriptors[] = {
//         { .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, .count = 1, .stage = VK_SHADER_STAGE_VERTEX_BIT },
//         { .type  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
//           .count = 1,
//           .stage = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT },
//         { .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, .count = 1, .stage = VK_SHADER_STAGE_VERTEX_BIT, .set = 1 },
//         { .type  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
//           .count = 1,
//           .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
//           .set   = 2 }
//     };
//
//     render::Pipeline pipeline{ context,
//                                render::ShaderStagesSpecification{ vertex_shader, fragment_shader, vertex_description },
//                                renderpass,
//                                binding_descriptors,
//                                { &push_constant, 1 } };
//
//     render::DescriptorPool descriptor_pool{ context };
//
//     PushConstantData push_constant_data{};
//     auto start_time = std::chrono::high_resolution_clock::now();
//
//     // TODO: remove this laziness
//     FrameDatas frame_datas;
//
//     render::resources::Buffer scene_data_buffer = render::resources::Buffer::start_build(
//                                                       "SceneDataBuffer",
//                                                       MAX_FRAMES * pad_uniform_buffer_size(context, sizeof(SceneData)))
//                                                       .usage(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT)
//                                                       //.allocation_type(VMA_MEMORY_USAGE_CPU_TO_GPU)
//                                                       .allocation_type(VMA_MEMORY_USAGE_AUTO)
//                                                       .allocation_flag(VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT)
//                                                       .build(context.allocator());
//
//     render::resources::BufferView buffer_view{ scene_data_buffer, 0, sizeof(SceneData) };
//
//     // initialize datas.
//     {
//         s32 num_images = swapchain.num_images();
//         auto [x, y]    = swapchain.extent();
//         for (s32 i = 0; i < num_images; ++i) {
//             auto& frame_data = frame_datas[i];
//
//             const auto uniform_buffer_name = fmt::format("Uniform buffer : {}", i);
//             const auto object_buffer_name  = fmt::format("Object buffer : {}", i);
//             frame_data.uniform_buffer = render::resources::Buffer::start_build<UniformBufferData>(uniform_buffer_name)
//                                             .usage(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT)
//                                             .allocation_type(VMA_MEMORY_USAGE_CPU_TO_GPU)
//                                             .build(context.allocator());
//
//             frame_data.object_storage_buffer = render::resources::Buffer::start_build<ObjectData>(object_buffer_name)
//                                                    .count(MAX_OBJECTS)
//                                                    .usage(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT)
//                                                    .allocation_type(VMA_MEMORY_USAGE_CPU_TO_GPU)
//                                                    .build(context.allocator());
//
//             frame_data.bindings = descriptor_pool.allocate(pipeline);
//
//             // clang-format off
//             frame_data.bindings.start_batch()
//                 .bind(0, frame_data.uniform_buffer,VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
//                 .bind(1, buffer_view, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC)
//                 // @EVALUATE : check if this good enough? { 1, 0 } maybe better.
//                 .bind(1, 0 , frame_data.object_storage_buffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER)
//                 .bind(2, 0, lost_empire, lost_empire_sampler)
//                 .end_batch();
//             // clang-format on
//             frame_data.command_buffer  = render::scene::CommandBuffer{ context, render::Operation::graphics };
//             frame_data.in_flight_fence = render::sync::Fence{ context, true };
//
//             frame_data.depth_buffer                    = create_depth_buffer(context, x, y);
//             const render::resources::TextureView* tv[] = { swapchain.get_image(i), &(frame_data.depth_buffer.view()) };
//             frame_data.render_target                   = render::Framebuffer{ context, renderpass, tv, x, y };
//         }
//     }
//
//     auto resize_fn = [&frame_datas, &context, &renderpass](render::Swapchain& sc, u32 x, u32 y) {
//         auto num_images = sc.num_images();
//         for (s32 i = 0; i < num_images; ++i) {
//             auto& frame_data                           = frame_datas[i];
//             frame_data.depth_buffer                    = create_depth_buffer(context, x, y);
//             const render::resources::TextureView* tv[] = { sc.get_image(i), &(frame_data.depth_buffer.view()) };
//             frame_data.render_target                   = render::Framebuffer{ context, renderpass, tv, x, y };
//         }
//     };
//
//     swapchain.on_resize(resize_fn);
//
//     upload_cmd_buffer.wait();
//
//     while (main_window.is_open()) {
//         // TODO: add frame data in.
//         const auto current_idx = swapchain.current_image();
//         auto& frame_data       = frame_datas[current_idx];
//
//         // wait for frame to begin.
//         frame_data.in_flight_fence.wait();
//         frame_data.in_flight_fence.reset();
//
//         glm::vec3 cam_pos    = { 0.f, -6.f, -10.f };
//         glm::mat4 view       = glm::translate(glm::mat4(1.f), cam_pos);
//         glm::mat4 projection = glm::perspective(glm::radians(70.f), 1700.f / 900.f, 0.1f, 200.0f);
//         projection[1][1] *= -1;
//
//         frame_data.uniform_buffer.map<UniformBufferData>([&](UniformBufferData* data) {
//             if (data) {
//                 data->view     = view;
//                 data->proj     = projection;
//                 data->viewproj = projection * view;
//             }
//         });
//
//         auto current_time = std::chrono::high_resolution_clock::now();
//         f32 time          = std::chrono::duration<f32, std::chrono::seconds::period>(current_time - start_time).count();
//         f32 var           = time * glm::radians(360.0f);
//
//         u32 offset = static_cast<u32>(current_idx * pad_uniform_buffer_size(context, sizeof(SceneData)));
//         render::resources::BufferView scene_data_buffer_view{ scene_data_buffer, offset, offset + sizeof(SceneData) };
//         scene_data_buffer_view.map<SceneData>([&](SceneData* data) {
//             if (data) data->ambient_color = { sin(var), 0, cos(var), 1 };
//         });
//
//         glm::mat4 model = // glm::mat4{ 1.0f };
//             glm::translate(glm::vec3{ 5, -10, 0 });
//
//         // glm::rotate(glm::mat4{ 1.0f }, time * glm::radians(90.0f), glm::vec3(0, 1, 0));
//         push_constant_data.render_matrix = model;
//         frame_data.object_storage_buffer.map<ObjectData>([&](ObjectData* data) {
//             if (data != nullptr) {
//                 data[0].model_mat = model;
//             }
//         });
//
//         auto& command_context     = frame_data.command_buffer;
//         const auto& viewport_info = swapchain.get_viewport_info();
//         command_context.set_viewport(viewport_info.viewport);
//         command_context.set_scissor(viewport_info.scissor);
//
//         VkClearValue depth_clear{};
//         depth_clear.depthStencil.depth = 1.f;
//         VkClearValue clear_color[]     = { { { { 0.1f, 0.1f, 0.1f, 1.0f } } }, depth_clear };
//         command_context.begin_renderpass(frame_data.render_target, clear_color);
//
//         command_context.bind_pipeline(pipeline)
//             .push_constants(push_constant, &push_constant_data)
//             .bindings(frame_data.bindings, { &offset, 1 });
//
//         command_context.bind_mesh(mesh);
//         command_context.draw_indexed(1);
//
//         command_context.end_renderpass();
//         command_context.submit(swapchain.current_present_context(), frame_data.in_flight_fence);
//         swapchain.present();
//
//         windows::poll_events();
//     }
//
//     // TODO: we can remove this after we find out how to properly tie
//     // resources to each frame.
//     context.wait();
//
//     return application::ExitStatus::ok;
// }



application::ExitStatus main(application::Settings args) noexcept {
    (void)args;
    return adapters::imgui::test();
}

} // namespace zoo
