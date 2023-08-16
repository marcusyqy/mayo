#include "Layer.hpp"
#include "adapters/imgui/Render.hpp"
#include "adapters/tools/ShaderCompiler.hpp"

#include "backends/imgui_impl_glfw.h"

#include "fonts/Roboto.hpp"
#include "render/scene/UploadContext.hpp"

namespace zoo::adapters::imgui {
    
    Layer::Layer(render::Engine& engine, Window& window) noexcept : engine_(&engine), window_(&window) {}

Layer::~Layer() noexcept {}

void Layer::init() noexcept {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    auto& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;     // Enable Docking
    // io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;   // Enable Multi-Viewport / Platform Windows
    // io.ConfigViewportsNoAutoMerge = true;
    // io.ConfigViewportsNoTaskBarIcon = true;

    // Setup Dear ImGui style
    
    ImGui::StyleColorsDark();


    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        style.WindowRounding              = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }



    // Load default font
    ImFontConfig font_config;
    font_config.FontDataOwnedByAtlas = false;
    ImFont* font   = io.Fonts->AddFontFromMemoryTTF((void*)roboto_font, sizeof(roboto_font), 15.0f, &font_config);
    io.FontDefault = font;

    // Probably Okay.
    ImGui_ImplGlfw_InitForVulkan(window_->impl(), true);
    imgui_init(*engine_, engine_->context(), window_->swapchain());
}

void Layer::exit() noexcept {
    imgui_exit();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void Layer::update() noexcept {
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    // Here is an example of some drawing needed.]
    static bool show_demo_window = true;

    {
        static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

        // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
        // because it would be confusing to have two docking targets within each others.
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking;
        window_flags |= ImGuiWindowFlags_MenuBar;

        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

        // When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
        // and handle the pass-thru hole, so we ask Begin() to not render a background.
        if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode) window_flags |= ImGuiWindowFlags_NoBackground;

        // Important: note that we proceed even if Begin() returns false (aka window is collapsed).
        // This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
        // all active windows docked into it will lose their parent and become undocked.
        // We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
        // any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("DockSpace", nullptr, window_flags);
        ImGui::PopStyleVar();

        ImGui::PopStyleVar(2);

        // Submit the DockSpace
        ImGuiIO& io = ImGui::GetIO();
        if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
            ImGuiID dockspace_id = ImGui::GetID("Zoo Dockspace");
            ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
        }

        if (ImGui::BeginMenuBar()) {
            if (ImGui::MenuItem("Demo")) {
                show_demo_window = true;
            }
            ImGui::EndMenuBar();
        }

        if(show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);

        ImGui::End();
    }

    // Here we need to have somewhere to add some drawing.
    ImGui::Render();
}

void Layer::render() noexcept {
    imgui_frame_render();

    auto& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }

    imgui_present();
}

application::ExitStatus test() {
    const application::Info app_context{ { 0, 0, 0 }, "Zoo::Application" };
    const render::engine::Info render_engine_info{ app_context, true };

    ZOO_LOG_INFO("Starting application");

    render::Engine render_engine{ render_engine_info };

    // TODO: I think we should just merge swapchain and window
    Window main_window{ render_engine,
                        window::Traits{ window::Size{ 1280, 960 }, false, "Zoo" },
                        [](Window& win, input::KeyCode keycode) {
                            if (keycode.key_ == input::Key::escape && keycode.action_ == input::Action::pressed) {
                                win.close();
                            }
                        } };
    Layer layer(render_engine, main_window);

    layer.init();

    while (main_window.is_open()) {
        layer.update();
        layer.render();
        windows::poll_events();
    }

    layer.exit();
    return application::ExitStatus::ok;

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
    //         f32 time          = std::chrono::duration<f32, std::chrono::seconds::period>(current_time -
    //         start_time).count(); f32 var           = time * glm::radians(360.0f);
    //
    //         u32 offset = static_cast<u32>(current_idx * pad_uniform_buffer_size(context, sizeof(SceneData)));
    //         render::resources::BufferView scene_data_buffer_view{ scene_data_buffer, offset, offset +
    //         sizeof(SceneData) }; scene_data_buffer_view.map<SceneData>([&](SceneData* data) {
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
    //
    //
}

} // namespace zoo::adapters::imgui