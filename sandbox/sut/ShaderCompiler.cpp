#include "ShaderCompiler.hpp"
#include "spdlog/spdlog.h"

namespace sut {
namespace {} // namespace

stdx::expected<std::vector<uint32_t>, std::runtime_error>
ShaderCompiler::compile(const ShaderWork& work) noexcept {
    shaderc::CompileOptions options;
    for (const auto& defines : work.defines) {
        options.AddMacroDefinition(defines.name, defines.value);
    }

    shaderc::SpvCompilationResult module = compiler_.CompileGlslToSpv(
        work.bytes, work.kind, work.name.c_str(), options);

    options.SetOptimizationLevel(shaderc_optimization_level_performance);

    if (module.GetCompilationStatus() != shaderc_compilation_status_success) {
        return stdx::unexpected{std::runtime_error(module.GetErrorMessage())};
    }

    return std::vector<uint32_t>{module.cbegin(), module.cend()};
}
} // namespace sut
