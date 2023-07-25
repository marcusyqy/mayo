#include "shader_compiler.hpp"
#include "core/fwd.hpp"
#include "spdlog/spdlog.h"

namespace zoo::adapters::tools {

stdx::expected<std::vector<u32>, std::runtime_error> Shader_Compiler::compile(const Shader_Work& work) noexcept {
    shaderc::CompileOptions options;
    for (const auto& defines : work.defines) {
        options.AddMacroDefinition(defines.name, defines.value);
    }

    shaderc::SpvCompilationResult module =
        compiler_.CompileGlslToSpv(work.bytes, work.kind, work.name.c_str(), options);

    options.SetOptimizationLevel(shaderc_optimization_level_performance);

    if (module.GetCompilationStatus() != shaderc_compilation_status_success) {
        return stdx::unexpected{ std::runtime_error(module.GetErrorMessage()) };
    }

    return std::vector<uint32_t>{ module.cbegin(), module.cend() };
}
} // namespace zoo::adapters::tools
