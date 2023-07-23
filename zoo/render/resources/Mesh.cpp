#include "Mesh.hpp"

#include "render/fwd.hpp"
#include <tiny_obj_loader.h>

#include "render/scene/CommandBuffer.hpp"
#include "render/sync/Fence.hpp"

namespace zoo::render::resources {

namespace {

// lifted from vkguide.dev
MeshData load_mesh_data(std::string_view file_name) {

    // attrib will contain the vertex arrays of the file
    tinyobj::attrib_t attrib;
    // shapes contains the info for each separate object in the file
    std::vector<tinyobj::shape_t> shapes;
    // materials contains the information about the material of each shape, but
    // we won't use it.
    std::vector<tinyobj::material_t> materials;

    // error and warning output from the load function
    std::string warn;
    std::string err;

    // load the OBJ file
    tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, file_name.data(), nullptr);

    if (!warn.empty()) {
        ZOO_LOG_WARN("[load_mesh] : {}", warn);
    }

    if (!err.empty()) {
        ZOO_LOG_ERROR("[load_mesh] : {}", err);
    }

    std::vector<Vertex> vertices{};
    std::vector<uint32_t> indices{};

    for (const auto& shape : shapes) {
        for (const auto& index : shape.mesh.indices) {
            Vertex vertex{};
            vertex.pos = { attrib.vertices[3 * index.vertex_index + 0],
                           attrib.vertices[3 * index.vertex_index + 1],
                           attrib.vertices[3 * index.vertex_index + 2] };

            vertex.normal = { attrib.normals[3 * index.normal_index + 0],
                              attrib.normals[3 * index.normal_index + 1],
                              attrib.normals[3 * index.normal_index + 2] };

            // TODO: change this part to white again
            vertex.color = vertex.normal; //  { 1.0, 1.0, 1.0 };
            vertex.uv    = { attrib.texcoords[2 * index.texcoord_index + 0],
                             1.0f - attrib.texcoords[2 * index.texcoord_index + 1] };

            vertices.push_back(vertex);
            indices.push_back(uint32_t(indices.size()));
        }
    }

    return { vertices, indices };
}

} // namespace

// out of the lack of anywhere else to put this.
std::array<VertexBufferDescription, 4> Vertex::describe() noexcept {
    return std::array{ VertexBufferDescription{ 0, render::ShaderType::vec3, offsetof(Vertex, pos) },
                       VertexBufferDescription{ 1, render::ShaderType::vec3, offsetof(Vertex, normal) },
                       VertexBufferDescription{ 2, render::ShaderType::vec3, offsetof(Vertex, color) },
                       VertexBufferDescription{ 3, render::ShaderType::vec2, offsetof(Vertex, uv) } };
}

template <typename T>
render::resources::Buffer create_gpu_native_buffer(
    std::string_view name,
    render::scene::CommandBuffer& cmd_buffer,
    stdx::span<T> variable,
    render::DeviceContext& context,
    VkBufferUsageFlags usage) {

    // TODO: add optional range.
    auto scratch_buffer = render::resources::Buffer::start_build<T>(fmt::format("ScratchBuffer : {}", name))
                              .count(static_cast<u32>(variable.size()))
                              .usage(VK_BUFFER_USAGE_TRANSFER_SRC_BIT)
                              .allocation_type(VMA_MEMORY_USAGE_AUTO_PREFER_HOST)
                              .allocation_flag(VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT)
                              .build(context.allocator());

    scratch_buffer.template map<T>([&variable](T* data) { std::copy(std::begin(variable), std::end(variable), data); });

    auto gpu_native_buffer = render::resources::Buffer::start_build<T>(name)
                                 .count(static_cast<u32>(variable.size()))
                                 .usage(VK_BUFFER_USAGE_TRANSFER_DST_BIT | usage)
                                 .allocation_type(VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE)
                                 .build(context.allocator());

    cmd_buffer.copy(scratch_buffer, gpu_native_buffer);

    render::sync::Fence fence{ context };
    cmd_buffer.submit(nullptr, nullptr, nullptr, fence);
    fence.wait();

    return gpu_native_buffer;
}

Mesh::Mesh(DeviceContext& context, scene::CommandBuffer& cmd_buffer, MeshData mesh_data, std::string_view name) noexcept
    :
    buffer_(create_gpu_native_buffer<Vertex>(
        name,
        cmd_buffer,
        mesh_data.vertices,
        context,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT)),
    index_buffer_(create_gpu_native_buffer<uint32_t>(
        name,
        cmd_buffer,
        mesh_data.indices,
        context,
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT)),
    data_(std::move(mesh_data)) {}

Mesh::Mesh(DeviceContext& context, scene::CommandBuffer& cmd_buffer, std::string_view file_name) noexcept :
    Mesh(context, cmd_buffer, load_mesh_data(file_name), file_name) {}

} // namespace zoo::render::resources
