
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

struct FrameData {
    render::resources::Buffer uniform_buffer;
    render::ResourceBindings bindings;
};

struct UniformBufferData {
    glm::mat4 view;
    glm::mat4 proj;
    glm::mat4 viewproj;
};

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

Shaders read_shaders() noexcept {
    render::tools::ShaderCompiler compiler;
    auto vertex_bytes = read_file("static/shaders/Test.vert");
    ZOO_ASSERT(vertex_bytes, "vertex shader must have value!");
    auto fragment_bytes = read_file("static/shaders/Test.frag");
    ZOO_ASSERT(fragment_bytes, "fragment shader must have value!");

    render::tools::ShaderWork vertex_work{ shaderc_vertex_shader, "Test.vert", *vertex_bytes };
    render::tools::ShaderWork fragment_work{ shaderc_fragment_shader, "Test.frag", *fragment_bytes };

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
                            if (keycode.key_ == input::Key::escape && keycode.action_ == input::Action::pressed) {
                                win.close();
                            }
                        } };

    auto [vertex_bytes, fragment_bytes] = read_shaders();
    render::Shader vertex_shader{ context, vertex_bytes, "main" };
    render::Shader fragment_shader{ context, fragment_bytes, "main" };

    render::resources::Mesh mesh{ context.allocator(), "static/assets/viking_room.obj" };

    auto& swapchain = main_window.swapchain();

    auto buffer_description = render::resources::Vertex::describe();
    std::array vertex_description{ render::VertexInputDescription{ sizeof(render::resources::Vertex),
                                                                   buffer_description,
                                                                   VK_VERTEX_INPUT_RATE_VERTEX } };

    render::PushConstant push_constant{};
    push_constant.size       = sizeof(PushConstantData);
    push_constant.offset     = 0;
    push_constant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    render::BindingDescriptor binding_descriptors[] = {
        { .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, .count = 1, .stage = VK_SHADER_STAGE_VERTEX_BIT }
    };

    render::Pipeline pipeline{ context,
                               render::ShaderStagesSpecification{ vertex_shader, fragment_shader, vertex_description },
                               swapchain.get_viewport_info(),
                               // TODO: this needs to change to a normal renderpass type that can be
                               // accessed from the outside AND created AND configured outside.
                               swapchain.get_renderpass(),
                               binding_descriptors,
                               &push_constant };

    render::DescriptorPool descriptor_pool{ context };

    PushConstantData push_constant_data{};
    auto start_time = std::chrono::high_resolution_clock::now();

    // TODO: remove this laziness
    FrameDatas frame_datas;

    // initialize datas.
    for (s32 i = 0; i < MAX_FRAMES; ++i) {
        auto& frame_data = frame_datas[i];

        const auto name           = fmt::format("Uniform buffer : {}", i);
        frame_data.uniform_buffer = render::resources::Buffer::start_build<UniformBufferData>(name)
                                        .count(1)
                                        .usage(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT)
                                        .allocation_type(VMA_MEMORY_USAGE_CPU_TO_GPU)
                                        .build(context.allocator());
        frame_data.bindings = descriptor_pool.allocate(pipeline);
        frame_data.bindings.start_batch().bind(0, frame_data.uniform_buffer).end_batch();
    }

    while (main_window.is_open()) {
        // TODO: add frame data in.

        const auto current_idx = swapchain.current_image();
        auto& frame_data       = frame_datas[current_idx];

        glm::mat4 view =
            glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        glm::mat4 projection = glm::perspective(glm::radians(70.f), 1700.f / 900.f, 0.1f, 200.0f);
        projection[1][1] *= -1;

        frame_data.uniform_buffer.map<UniformBufferData>([&](UniformBufferData* data) {
            if (data) {
                data->view     = view;
                data->proj     = projection;
                data->viewproj = projection * view;
            }
        });

        auto current_time = std::chrono::high_resolution_clock::now();
        f32 time          = std::chrono::duration<f32, std::chrono::seconds::period>(current_time - start_time).count();

        glm::mat4 model = glm::rotate(glm::mat4{ 1.0f }, time * glm::radians(90.0f), glm::vec3(0, 0, 1));
        push_constant_data.render_matrix = model;

        auto populate_command_ctx = [&](render::scene::CommandBuffer& command_context,
                                        VkRenderPassBeginInfo renderpass_info) {
            const auto& viewport_info = swapchain.get_viewport_info();
            command_context.set_viewport(viewport_info.viewport);
            command_context.set_scissor(viewport_info.scissor);

            command_context.exec(renderpass_info, [&]() {
                command_context.bind_pipeline(pipeline)
                    .push_constants(push_constant, &push_constant_data)
                    .bindings(frame_data.bindings);

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
