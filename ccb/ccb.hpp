#pragma once

#include <string>

namespace ccb {

enum class Architecture {
    UNIVERSAL, // The universal binaries supported by iOS and macOS
    x86,
    x86_64,
    ARM,
    ARM64,
};

enum class Platform { WINDOWS, LINUX, MAC };
enum class Compiler { CLANG, MICROSOFT, GCC };

#ifdef WIN32
struct Query {
    constexpr static Platform platform         = Platform::WINDOWS;
    constexpr static Compiler default_compiler = Compiler::MICROSOFT;
};
#else
static_assert(false, "OTHER PLATFORMS OTHER THAN WINDOWS NOT SUPPORTED YET");
#endif

template <Compiler compiler = Query::default_compiler>
struct Workspace {
    std::string name;
    Architecture architecture;
};

} // namespace ccb
