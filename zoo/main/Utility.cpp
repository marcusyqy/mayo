#include "Utility.hpp"

#include "core/fwd.hpp"

#include "render/resources/Mesh.hpp"
#include <fstream>

namespace zoo {

stdx::expected<std::string, std::runtime_error> read_file(
    std::string_view filename) noexcept {
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
void populate_vertices(render::DeviceContext& context) {
    const std::vector<render::resources::Vertex> vertices = {
        { { -0.5f, -0.5f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f } },
        { { 0.5f, -0.5f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f } },
        { { 0.5f, 0.5f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } },
        { { -0.5f, 0.5f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } }
    };

    // we try to use uint32_t for indices
    const std::vector<u32> indices = { 0, 1, 2, 2, 3, 0 };
    auto vertex_buffer =
        render::resources::Buffer::start_build<render::resources::Vertex>(
            "entry point vertex buffer")
            .usage(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT)
            .allocation_type(VMA_MEMORY_USAGE_CPU_TO_GPU)
            .count(vertices.size())
            .build(context.allocator());

    vertex_buffer.map<render::resources::Vertex>(
        [&vertices](render::resources::Vertex* data) {
            std::copy(std::begin(vertices), std::end(vertices), data);
        });

    auto index_buffer = render::resources::Buffer::start_build<uint32_t>(
        "entry point index buffer")
                            .usage(VK_BUFFER_USAGE_INDEX_BUFFER_BIT)
                            .allocation_type(VMA_MEMORY_USAGE_CPU_TO_GPU)
                            .count(indices.size())
                            .build(context.allocator());

    index_buffer.map<uint32_t>([&indices](uint32_t* data) {
        std::copy(std::begin(indices), std::end(indices), data);
    });
}
} // namespace example

} // namespace zoo
