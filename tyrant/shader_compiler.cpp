#include "shader_compiler.hpp"
#include "basic.hpp"
#include <utility>

Compiled_Shader Shader_Compiler::compile(const Shader_Work& work) noexcept {
    shaderc_compile_options_t options = shaderc_compile_options_initialize();
    defer { shaderc_compile_options_release(options); };

    for (const auto& defines : work.defines) {
        shaderc_compile_options_add_macro_definition(
            options,
            defines.name.data,
            defines.name.count,
            defines.value.data,
            defines.value.count);
    }

    shaderc_compile_options_set_optimization_level(options, shaderc_optimization_level_performance);

    shaderc_compilation_result_t compilation_result = shaderc_compile_into_spv(
        compiler,
        work.bytes.data,
        work.bytes.count,
        work.kind,
        work.name.data,
        "main", // entry point
        options);

    return { compilation_result };
}

Compiled_Shader::operator bool() const {
    return result && shaderc_result_get_compilation_status(result) == shaderc_compilation_status_success;
}

const u32* Compiled_Shader::data() const { return reinterpret_cast<const u32*>(shaderc_result_get_bytes(result)); }
size_t Compiled_Shader::size_in_bytes() const { return shaderc_result_get_length(result); }
size_t Compiled_Shader::size_in_elements() const { return size_in_bytes() / sizeof(u32); }

// shader_compiler?
void Shader_Compiler::free(Compiled_Shader& shader) { shaderc_result_release(shader.result); }

Shader_Compiler create_shader_compiler() { return { shaderc_compiler_initialize() }; }
void free_shader_compiler(Shader_Compiler& compiler) { shaderc_compiler_release(compiler.compiler); }
