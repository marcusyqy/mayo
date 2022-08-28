#include "render/Engine.hpp"

namespace zoo::render {

void Engine::initialize() noexcept {}

void Engine::cleanup() noexcept {
    if (instance_ != nullptr) {
        vkDestroyInstance(instance_, nullptr);
    }
}

void Engine::create_instance() noexcept {}

void Engine::create_device() noexcept {}

} // namespace zoo::render