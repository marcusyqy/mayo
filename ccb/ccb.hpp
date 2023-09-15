#pragma once

#include <string>
#include <vector>

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
enum class Workspace_Flags : unsigned int { NONE = 0, MULTI_PROCESSOR_COMPILE = 1 << 0, ALL = 0xFFFFFFFF };

// refine this platform stuff.
#ifdef _WIN32
struct Query {
    constexpr static Platform platform         = Platform::WINDOWS;
    constexpr static Compiler default_compiler = Compiler::MICROSOFT;
};

#else
#ifdef __linux__
static_assert(false, "OTHER PLATFORMS OTHER THAN WINDOWS NOT SUPPORTED YET");
#else
static_assert(false, "OTHER PLATFORMS OTHER THAN WINDOWS NOT SUPPORTED YET");
#endif
#endif

// __linux__       Defined on Linux
// __sun           Defined on Solaris
// __FreeBSD__     Defined on FreeBSD
// __NetBSD__      Defined on NetBSD
// __OpenBSD__     Defined on OpenBSD
// __APPLE__       Defined on Mac OS X
// __hpux          Defined on HP-UX
// __osf__         Defined on Tru64 UNIX (formerly DEC OSF1)
// __sgi           Defined on Irix
// _AIX            Defined on AIX
// _WIN32          Defined on Windows

template <Compiler compiler = Query::default_compiler>
struct Workspace {
    std::string name                        = {};
    Architecture architecture               = Architecture::x86_64;
    Workspace_Flags flags                   = Workspace_Flags::NONE;
    std::vector<std::string> configurations = {};
    std::string output_dir                  = {};
};

using Default_Workspace = Workspace<>;

} // namespace ccb
