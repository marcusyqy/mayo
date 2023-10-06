#include "utils.hpp"
#include "core/fwd.hpp"
#include "render/resources/mesh.hpp"
#include <fstream>

namespace zoo::core {

void check_memory() noexcept {
#if defined(WIN32)
#if defined(DEBUG) | defined(_DEBUG)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
#endif
}

stdx::expected<std::string, std::runtime_error> read_file(std::string_view filename) noexcept {
    std::ifstream file{ filename.data(), std::ios::ate | std::ios::binary };
    if (!file.is_open()) {
        return stdx::unexpected{ std::runtime_error("unable to open file!") };
    }

    const auto file_size = static_cast<u64>(file.tellg());
    std::string buffer;
    buffer.resize(file_size);
    file.seekg(0);
    file.read(buffer.data(), file_size);
    file.close();
    return buffer;
}

namespace example {
void populate_vertices(render::Device_Context& context) {
    const std::vector<render::resources::Vertex> vertices = {
        { { -0.5f, -0.5f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f } },
        { { 0.5f, -0.5f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f } },
        { { 0.5f, 0.5f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } },
        { { -0.5f, 0.5f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } }
    };

    // we try to use uint32_t for indices
    const std::vector<u32> indices = { 0, 1, 2, 2, 3, 0 };
    auto vertex_buffer = render::resources::Buffer::start_build<render::resources::Vertex>("entry point vertex buffer")
                             .usage(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT)
                             .allocation_type(VMA_MEMORY_USAGE_CPU_TO_GPU)
                             .count(vertices.size())
                             .build(context.allocator());

    vertex_buffer.map<render::resources::Vertex>(
        [&vertices](render::resources::Vertex* data) { std::copy(std::begin(vertices), std::end(vertices), data); });

    auto index_buffer = render::resources::Buffer::start_build<uint32_t>("entry point index buffer")
                            .usage(VK_BUFFER_USAGE_INDEX_BUFFER_BIT)
                            .allocation_type(VMA_MEMORY_USAGE_CPU_TO_GPU)
                            .count(indices.size())
                            .build(context.allocator());

    index_buffer.map<uint32_t>([&indices](uint32_t* data) { std::copy(std::begin(indices), std::end(indices), data); });
}
} // namespace example
} // namespace zoo::core
//
//

namespace zoo {

void* ptr_round_up_align(void* ptr, uintptr_t align) noexcept {
    ZOO_ASSERT(is_power_of_two(align), "align must be a power of two!");
    return (void*)(((uintptr_t)ptr + (align - 1)) & ~(align - 1));
}

void* ptr_round_down_align(void* ptr, uintptr_t align) noexcept {
    ZOO_ASSERT(is_power_of_two(align), "align must be a power of two!");
    return (void*)((uintptr_t)ptr & ~(align - 1));
}
} // namespace zoo
