#include "stdx/span.hpp"

#include <fstream>
#include <optional>

#include "spdlog/spdlog.h"
#include "sut/shader_compiler.hpp"

stdx::expected<std::string, std::runtime_error> read_file(std::string_view filename) noexcept {
    std::ifstream file{ filename.data(), std::ios::ate | std::ios::binary };
    if (!file.is_open()) {
        return stdx::unexpected{ std::runtime_error("unable to open file!") };
    }

    const std::size_t file_size = static_cast<size_t>(file.tellg());
    std::string buffer;
    buffer.resize(file_size);
    file.seekg(0);
    file.read(buffer.data(), file_size);
    file.close();
    return buffer;
}

int main() {
    spdlog::info("hello world");
    auto vertex_bytes = read_file("static/shaders/test.vert");
    assert(vertex_bytes && "vertex shader must have value!");
    auto fragment_bytes = read_file("static/shaders/test.frag");
    assert(fragment_bytes && "fragment shader must have value!");

    sut::ShaderCompiler compiler{};

    sut::ShaderWork vert_shader{ shaderc_glsl_vertex_shader, "vertex shader", *vertex_bytes };
    sut::ShaderWork frag_shader{ shaderc_glsl_fragment_shader, "fragment shader", *fragment_bytes };

    auto vertex_spirv   = compiler.compile(vert_shader);
    auto fragment_spirv = compiler.compile(frag_shader);

    if (!vertex_spirv) {
        spdlog::error("error from vertex spirv : {}", vertex_spirv.error().what());
    } else {
        std::fstream fs;
        fs.open("static/shaders/vert.spv", std::fstream::trunc | std::fstream::out);
        auto& spirv = vertex_spirv.value();
        fs.write(reinterpret_cast<const char*>(spirv.data()), sizeof(uint32_t) * spirv.size());
    }

    if (!fragment_spirv) {
        spdlog::error("error from fragment spirv : {}", fragment_spirv.error().what());
    } else {

        std::fstream fs;
        fs.open("static/shaders/frag.spv", std::fstream::trunc | std::fstream::out);
        auto& spirv = fragment_spirv.value();
        fs.write(reinterpret_cast<const char*>(spirv.data()), sizeof(uint32_t) * spirv.size());
    }
}
