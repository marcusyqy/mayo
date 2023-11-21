#pragma once
#include "log.hpp"

#define GL_SILENCE_DEPRECATION
#include "backends/imgui_impl_glfw.h"
#include <GLFW/glfw3.h> // Will drag system OpenGL headers
#include <imgui.h>
#include "basic.hpp"


GLFWwindow* create_window(s32 width, s32 height, const char* name);

