#pragma once
#include "string.hpp"
#include "types.hpp"
#include <shaderc/shaderc.h>

struct Shader_Def_Type {
    String_View name;
    String_View value;
};

struct Shader_Work {
    shaderc_shader_kind kind;
    String_View name;
    String_View bytes;
    Buffer_View<Shader_Def_Type> defines = {};
};

struct Compiled_Shader {
    operator bool() const;
    const u32* data() const;
    size_t size_in_elements() const;
    size_t size_in_bytes() const;

    // mem
    shaderc_compilation_result_t result;
};

struct Shader_Compiler {
    Compiled_Shader compile(const Shader_Work& work) noexcept;
    void free(Compiled_Shader& shader);

    // mem
    shaderc_compiler_t compiler;
};

// shader compiler
Shader_Compiler create_shader_compiler();
void free_shader_compiler(Shader_Compiler& compiler);
