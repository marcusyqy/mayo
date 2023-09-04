#pragma once
#include "imgui.h" // IMGUI_IMPL_API

// Forward declaration.
struct GLFWwindow;
struct GLFWmonitor;

namespace zoo::imgui {

// @TODO: use windows appropriately here
bool imgui_window_init(GLFWwindow* window, bool install_callbacks);
void imgui_window_new_frame();
void imgui_window_exit();
GLFWwindow* imgui_window_handle_from_viewport(ImGuiViewport* viewport);

} // namespace zoo::adapters::imgui
