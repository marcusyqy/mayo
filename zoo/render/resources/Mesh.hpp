#pragma once

#include <glm/glm.hpp>
#include <stdx/span.hpp>
#include <string_view>

#include "render/Pipeline.hpp"
#include "render/resources/Allocator.hpp"
#include "render/resources/Buffer.hpp"

namespace zoo::render::resources {

struct Vertex {
    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec3 color;

    // out of the lack of anywhere else to put this.
    static std::array<VertexBufferDescription, 3> describe() noexcept;
};

class Mesh {
public:
    Mesh(Allocator& allocator, std::vector<Vertex> vertices) noexcept;
    Mesh(Allocator& allocator, std::string_view file_name) noexcept;
    Mesh(Allocator& allocator, const char* file_name) noexcept : Mesh(allocator, std::string_view(file_name)){};

    operator const Buffer&() const noexcept { return buffer_; }
    const Buffer& get() const noexcept { return buffer_; }

    size_t count() const noexcept { return vertices_.size(); }


private:
    Buffer buffer_;
    std::vector<Vertex> vertices_;
};

} // namespace zoo::render::resources
