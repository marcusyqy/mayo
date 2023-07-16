
#include "EntryPoint.hpp"

#include "Utility.hpp"

#include "core/Array.hpp"
#include "core/Log.hpp"
#include "core/platform/Window.hpp"

#include "render/DescriptorPool.hpp"
#include "render/Engine.hpp"
#include "render/Pipeline.hpp"
#include "render/resources/Buffer.hpp"
#include "render/resources/Mesh.hpp"
#include "render/scene/CommandBuffer.hpp"

#include "stdx/expected.hpp"

#include "render/tools/ShaderCompiler.hpp"
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#include <array>
#include <fstream>
#include <string_view>

namespace zoo {

namespace {

render::resources::Texture load_house_texture(render::resources::Allocator& allocator, std::string_view name) noexcept {
    //TODO: some hacks were added here that should be removed when we eventually change this.
    // load texture here.
    const auto x = 1, y = 1;
    (void)name;

    return render::resources::Texture::start_build("House Texture")
        .format(VK_FORMAT_UNDEFINED)
        .usage(VK_IMAGE_USAGE_SAMPLED_BIT)
        .extent({ x, y, 1 })
        .allocation_type(VMA_MEMORY_USAGE_GPU_ONLY)
        .allocation_required_flags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
        .build(allocator);
}

struct FrameData {};


constexpr s32 MAX_FRAMES = 3;

using FrameDatas = FrameData[MAX_FRAMES];

struct PushConstantData {
    glm::vec4 data;
    glm::mat4 render_matrix;
};

using ShaderBytes = std::vector<u32>;

struct Shaders {
    ShaderBytes vertex;
    ShaderBytes fragment;
};

Shaders read_shaders() {
    render::tools::ShaderCompiler compiler;
    auto vertex_bytes = read_file("static/shaders/Test.vert");
    ZOO_ASSERT(vertex_bytes, "vertex shader must have value!");
    auto fragment_bytes = read_file("static/shaders/Test.frag");
    ZOO_ASSERT(fragment_bytes, "fragment shader must have value!");

    render::tools::ShaderWork vertex_work{ shaderc_vertex_shader, "Test.vert",
        *vertex_bytes };

    render::tools::ShaderWork fragment_work{ shaderc_fragment_shader,
        "Test.frag", *fragment_bytes };

    auto vertex_spirv = compiler.compile(vertex_work);
    auto fragment_spirv = compiler.compile(fragment_work);
    if (!vertex_spirv) {
        spdlog::error("Vertex has error : {}", vertex_spirv.error().what());
    }

    if (!fragment_spirv) {
        spdlog::error("Fragment has error : {}", fragment_spirv.error().what());
    }

    return { .vertex = std::move(*vertex_spirv),
        .fragment = std::move(*fragment_spirv) };
}

} // namespace

application::ExitStatus main(application::Settings args) noexcept {
    // TODO: to make runtime arguments for different stuff.
    (void)args;

    const application::Info app_context{ { 0, 0, 0 }, "Zoo::Application" };
    const render::engine::Info render_engine_info{ app_context, true };

    ZOO_LOG_INFO("Starting application");

    render::Engine render_engine{ render_engine_info };
    auto& context = render_engine.context();

    // TODO: I think we should just merge swapchain and window
    Window main_window{ render_engine,
        window::Traits{ window::Size{ 1280, 960 }, false, "Zoo" },
        [](Window& win, input::KeyCode keycode) {
            if (keycode.key_ == input::Key::escape &&
                keycode.action_ == input::Action::pressed) {
                win.close();
            }
        } };

    auto [vertex_bytes, fragment_bytes] = read_shaders();
    render::Shader vertex_shader{ context, vertex_bytes, "main" };
    render::Shader fragment_shader{ context, fragment_bytes, "main" };

    render::resources::Mesh mesh{ context.allocator(),
        "static/assets/viking_room.obj" };

    auto& swapchain = main_window.swapchain();

    auto buffer_description = render::resources::Vertex::describe();
    std::array vertex_description = { render::VertexInputDescription{
        sizeof(render::resources::Vertex), buffer_description,
        VK_VERTEX_INPUT_RATE_VERTEX } };

    render::PushConstant push_constant{};
    push_constant.size = sizeof(PushConstantData);
    push_constant.offset = 0;
    push_constant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    // TODO: find a way to abstract binding descriptors + pool and allow adding
    // them into pipeline. probably by creating a descriptor pool to be passed
    // into the pipeline that has the layout and some additional stuff. see
    // Pipeline.hpp description for more information.
    render::BindingDescriptor binding_descriptor = {
        .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .count = 1,
        .stage = VK_SHADER_STAGE_VERTEX_BIT
    };

    render::Pipeline pipeline{ context,
        render::ShaderStagesSpecification{
            vertex_shader, fragment_shader, vertex_description },
        swapchain.get_viewport_info(),
        // TODO: this needs to change to a normal renderpass type that can be
        // accessed from the outside AND created AND configured outside.
        swapchain.get_renderpass(), &binding_descriptor, &push_constant };

    render::DescriptorPool descriptor_pool{ context };

    PushConstantData push_constant_data{};
    auto start_time = std::chrono::high_resolution_clock::now();

    // TODO: remove this laziness
    FrameDatas frame_datas = {};

    while (main_window.is_open()) {
        // TODO: add frame data in.
        auto populate_command_ctx = [&](render::scene::CommandBuffer&
                                            command_context,
                                        VkRenderPassBeginInfo renderpass_info) {
            const auto& viewport_info = swapchain.get_viewport_info();
            command_context.set_viewport(viewport_info.viewport);
            command_context.set_scissor(viewport_info.scissor);

            command_context.exec(renderpass_info, [&]() {
                glm::mat4 view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f),
                    glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
                glm::mat4 projection = glm::perspective(
                    glm::radians(70.f), 1700.f / 900.f, 0.1f, 200.0f);
                projection[1][1] *= -1;

                auto current_time = std::chrono::high_resolution_clock::now();
                f32 time =
                    std::chrono::duration<f32, std::chrono::seconds::period>(
                        current_time - start_time)
                        .count();

                glm::mat4 model = glm::rotate(glm::mat4{ 1.0f },
                    time * glm::radians(90.0f), glm::vec3(0, 0, 1));

                // calculate final mesh matrix
                glm::mat4 mesh_matrix = projection * view * model;
                push_constant_data.render_matrix = mesh_matrix;

                command_context.bind_pipeline(pipeline).push_constants(
                    push_constant, &push_constant_data);
                command_context.bind_mesh(mesh);
                command_context.draw_indexed(1);
            });
        };

        swapchain.render(populate_command_ctx);
        swapchain.present();

        windows::poll_events();
    }

    // TODO: we can remove this after we find out how to properly tie
    // resources to each frame.
    context.wait();

    return application::ExitStatus::ok;
}

} // namespace zoo
