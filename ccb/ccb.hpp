#pragma once

// @TODO: evaluate if we want to use stl or not.
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
enum class Compiler { CLANG, MICROSOFT, GCC, CC };
enum class Cpp_Version { _11, _14, _17, _20, latest = _20 };
enum class Workspace_Flags : unsigned int { NONE = 0, MULTI_PROCESSOR_COMPILE = 1 << 0, ALL = 0xFFFFFFFF };

struct Query {
// refine this platform stuff.
#ifdef _WIN32
    // do i need to care about this?
    constexpr static Platform platform = Platform::WINDOWS;
    constexpr static Compiler compiler =
#if defined(__GNUC__)
        Compiler::GCC;
#elif defined(__clang__)
        Compiler::CLANG;
#elif defined(_MSC_VER)
        Compiler::MICROSOFT;
#endif
#endif

#ifdef __linux__
    static_assert(false, "OTHER PLATFORMS OTHER THAN WINDOWS NOT SUPPORTED YET");
#endif

#ifdef __APPLE__
    static_assert(false, "OTHER PLATFORMS OTHER THAN WINDOWS NOT SUPPORTED YET");
#endif
};

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

struct Run_Tree {
    const char* file_dir = "run_tree";
};

struct Build_Tree {
    const char* file_dir = ".ccb";
};

struct Project {
    const char* name;
    Run_Tree run_tree     = {};
    Build_Tree build_tree = {};

    std::vector<std::string> files = {};
    Cpp_Version cpp_version        = Cpp_Version::latest;
};

struct Source_Location {
    const char* file_name;
    // I don't need any of this information now
    // const char* function_name;
    // const int line;
    // const int column;

    static constexpr Source_Location current(const char* file_name = __builtin_FILE()) noexcept {
        return { file_name };
    }
};

struct Workspace {
    const char* name                        = {};
    Architecture architecture               = Architecture::x86_64;
    Workspace_Flags flags                   = Workspace_Flags::NONE;
    std::vector<std::string> configurations = {};
    Cpp_Version cpp_version                 = Cpp_Version::latest; // using latest.
    std::vector<Project> projects           = {};
};

namespace detail {

void ccb_internal_build(const char* source_path, int argc, char** argv) {
    assert(argc >= 1);
    const char* binary_path = argv[0];

    int rebuild_is_needed = nob_needs_rebuild(binary_path, &source_path, 1);
    if (rebuild_is_needed < 0) exit(1);
    if (rebuild_is_needed) {
        Nob_String_Builder sb = { 0 };
        nob_sb_append_cstr(&sb, binary_path);
        nob_sb_append_cstr(&sb, ".old");
        nob_sb_append_null(&sb);

        if (!nob_rename(binary_path, sb.items)) exit(1);
        Nob_Cmd rebuild = { 0 };
        nob_cmd_append(&rebuild, NOB_REBUILD_URSELF(binary_path, source_path));
        bool rebuild_succeeded = nob_cmd_run_sync(rebuild);
        nob_cmd_free(rebuild);
        if (!rebuild_succeeded) {
            nob_rename(sb.items, binary_path);
            exit(1);
        }

        Nob_Cmd cmd = { 0 };
        nob_da_append_many(&cmd, argv, argc);
        if (!nob_cmd_run_sync(cmd)) exit(1);
        exit(0);
    }
}

} // namespace detail

} // namespace ccb

#define BUILD_CCB(candidate, argc, argv) ccb_internal_build(__FILE__, argc, argv)
