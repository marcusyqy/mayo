#pragma once

#include "stdx/expected.hpp"
#include "stdx/span.hpp"
#include <shaderc/shaderc.hpp>
#include <string_view>

namespace sut {

struct shader_define_type {
    std::string name;
    std::string value;
};

struct shader_work {
    shaderc_shader_kind kind;
    std::string name;
    std::string bytes;
    stdx::span<shader_define_type> defines = {};
};

class shader_compiler {
public:
    using define_type = shader_define_type;
    stdx::expected<std::vector<uint32_t>, std::runtime_error> compile(
        const shader_work& work) noexcept;

private:
    shaderc::Compiler compiler_;
};
} // namespace sut
