#include "layer.hpp"
#include "adapters/imgui/render.hpp"
#include "adapters/imgui/window.hpp"
#include "adapters/tools/shader_compiler.hpp"

// Using a non hpp so that it doesn't kill lsp.
#include "fonts/roboto.embed"

#include "render/scene/upload_context.hpp"

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
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;   // Enable Multi-Viewport / Platform Windows
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
    imgui_window_init(window_->impl(), true);
    imgui_render_init(*engine_, engine_->context(), *window_);
}

void Layer::exit() noexcept {
    imgui_render_exit();
    imgui_window_exit();
    ImGui::DestroyContext();
}

void Layer::update() noexcept {
    imgui_window_new_frame();
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

        if (show_demo_window) ImGui::ShowDemoWindow(&show_demo_window);

        ImGui::End();
    }

    // Here we need to have somewhere to add some drawing.
    ImGui::Render();
}

void Layer::render() noexcept {
    imgui_render_frame_render();

    auto& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }

    imgui_render_present();
    window_->swap_buffers();
}

} // namespace zoo::adapters::imgui
