#pragma once

#include <glm/glm.hpp>
#include <stdx/span.hpp>
#include <string_view>

#include "render/Pipeline.hpp"
#include "render/resources/Allocator.hpp"
#include "render/resources/Buffer.hpp"

namespace zoo::render::scene {
class CommandBuffer;
}

namespace zoo::render::resources {

struct Vertex {
    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec3 color;
    glm::vec2 uv;

    // out of the lack of anywhere else to put this.
    static std::array<VertexBufferDescription, 4> describe() noexcept;
};

struct MeshData {
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
};

class Mesh {
public:
    Mesh(DeviceContext& context, scene::CommandBuffer& cmd_buffer, MeshData mesh_data, std::string_view name) noexcept;
    Mesh(DeviceContext& context, scene::CommandBuffer& cmd_buffer, std::string_view file_name) noexcept;
    Mesh(DeviceContext& context, scene::CommandBuffer& cmd_buffer, const char* file_name) noexcept :
        Mesh(context, cmd_buffer, std::string_view(file_name)){};

    Mesh(const Mesh& other)            = delete;
    Mesh& operator=(const Mesh& other) = delete;

    const Buffer& vertices() const noexcept { return buffer_; }
    const Buffer& indices() const noexcept { return index_buffer_; }

    size_t count() const noexcept { return data_.vertices.size(); }

private:
    Buffer buffer_;
    Buffer index_buffer_;
    MeshData data_;
};

} // namespace zoo::render::resources
