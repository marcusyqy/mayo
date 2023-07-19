#pragma once

#include "stdx/expected.hpp"
#include "stdx/span.hpp"
#include <shaderc/shaderc.hpp>
#include <string_view>

namespace sut {

struct ShaderDefType {
    std::string name;
    std::string value;
};

struct ShaderWork {
    shaderc_shader_kind kind;
    std::string name;
    std::string bytes;
    stdx::span<ShaderDefType> defines = {};
};

class ShaderCompiler {
public:
    using define_type = ShaderDefType;
    stdx::expected<std::vector<uint32_t>, std::runtime_error> compile(const ShaderWork& work) noexcept;

private:
    shaderc::Compiler compiler_;
};
} // namespace sut
