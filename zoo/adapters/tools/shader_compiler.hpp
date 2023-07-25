#pragma once

#include "core/fwd.hpp"
#include "stdx/expected.hpp"
#include "stdx/span.hpp"
#include <shaderc/shaderc.hpp>
#include <string_view>

namespace zoo::adapters::tools {

struct Shader_Def_Type {
    std::string name;
    std::string value;
};

struct Shader_Work {
    shaderc_shader_kind kind;
    std::string name;
    std::string bytes;
    stdx::span<Shader_Def_Type> defines = {};
};

class Shader_Compiler {
public:
    using define_type = Shader_Def_Type;
    stdx::expected<std::vector<u32>, std::runtime_error> compile(const Shader_Work& work) noexcept;

private:
    shaderc::Compiler compiler_;
};
} // namespace zoo::adapters::tools
