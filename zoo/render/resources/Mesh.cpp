#include "Mesh.hpp"

#include "render/fwd.hpp"
#include <tiny_obj_loader.h>

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
    tinyobj::LoadObj(
        &attrib, &shapes, &materials, &warn, &err, file_name.data(), nullptr);

    if (!warn.empty()) {
        ZOO_LOG_WARN("[load_mesh] : {}", warn);
    }

    if (!err.empty()) {
        ZOO_LOG_ERROR("[load_mesh] : {}", err);
    }

    std::vector<Vertex> vertices{};
    std::vector<uint32_t> indices{};
    // Loop over shapes
    // for (size_t s = 0; s < shapes.size(); s++) {
    //     // Loop over faces(polygon)
    //     size_t index_offset = 0;
    //     for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++)
    //     {
    //
    //         // hardcode loading to triangles
    //         size_t fv = 3;
    //
    //         // Loop over vertices in the face.
    //         for (size_t v = 0; v < fv; v++) {
    //             // access to vertex
    //             tinyobj::index_t idx = shapes[s].mesh.indices[index_offset +
    //             v];
    //
    //             // vertex position
    //             tinyobj::real_t vx = attrib.vertices[3 * idx.vertex_index +
    //             0]; tinyobj::real_t vy = attrib.vertices[3 * idx.vertex_index
    //             + 1]; tinyobj::real_t vz = attrib.vertices[3 *
    //             idx.vertex_index + 2];
    //             // vertex normal
    //             tinyobj::real_t nx = attrib.normals[3 * idx.normal_index +
    //             0]; tinyobj::real_t ny = attrib.normals[3 * idx.normal_index
    //             + 1]; tinyobj::real_t nz = attrib.normals[3 *
    //             idx.normal_index + 2];
    //
    //             // copy it into our vertex
    //             Vertex new_vert;
    //             new_vert.pos.x = vx;
    //             new_vert.pos.y = vy;
    //             new_vert.pos.z = vz;
    //
    //             new_vert.normal.x = nx;
    //             new_vert.normal.y = ny;
    //             new_vert.normal.z = nz;
    //
    //             // we are setting the vertex color as the vertex normal. This
    //             is
    //             // just for display purposes
    //             new_vert.color = new_vert.normal;
    //
    //             vertices.push_back(new_vert);
    //         }
    //         index_offset += fv;
    //     }
    // }
    for (const auto& shape : shapes) {
        for (const auto& index : shape.mesh.indices) {
            Vertex vertex{};
            vertex.pos = {attrib.vertices[3 * index.vertex_index + 0],
                attrib.vertices[3 * index.vertex_index + 1],
                attrib.vertices[3 * index.vertex_index + 2]};

            vertex.normal = {attrib.normals[3 * index.normal_index + 0],
                attrib.normals[3 * index.normal_index + 1],
                attrib.normals[3 * index.normal_index + 2]};

            vertex.color = vertex.normal;

            vertices.push_back(vertex);
            indices.push_back(indices.size());
        }
    }

    return {vertices, indices};
}

} // namespace

// out of the lack of anywhere else to put this.
std::array<VertexBufferDescription, 3> Vertex::describe() noexcept {
    return std::array{VertexBufferDescription{
                          0, render::ShaderType::vec3, offsetof(Vertex, pos)},
        VertexBufferDescription{
            1, render::ShaderType::vec3, offsetof(Vertex, normal)},
        VertexBufferDescription{
            2, render::ShaderType::vec3, offsetof(Vertex, color)}};
}

Mesh::Mesh(
    Allocator& allocator, MeshData mesh_data, std::string_view name) noexcept
    : buffer_(Buffer::start_build(allocator, name)
                  .usage(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT)
                  .allocation_type(VMA_MEMORY_USAGE_CPU_TO_GPU)
                  .size(sizeof(Vertex) * mesh_data.vertices.size())
                  .build()),
      index_buffer_(Buffer::start_build(allocator, name)
                        .usage(VK_BUFFER_USAGE_INDEX_BUFFER_BIT)
                        .allocation_type(VMA_MEMORY_USAGE_CPU_TO_GPU)
                        .size(sizeof(uint32_t) * mesh_data.indices.size())
                        .build()),
      data_(std::move(mesh_data)) {

    buffer_.map<Vertex>([this](Vertex* data) {
        std::copy(
            std::begin(data_.vertices), std::end(data_.vertices), data);
    });

    index_buffer_.map<uint32_t>([this](uint32_t* data) {
        std::copy(
            std::begin(data_.indices), std::end(data_.indices), data);
    });
}

Mesh::Mesh(Allocator& allocator, std::string_view file_name) noexcept
    : Mesh(allocator, load_mesh_data(file_name), file_name) {}

} // namespace zoo::render::resources