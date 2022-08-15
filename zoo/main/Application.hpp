#pragma once
#include <cstdint>

namespace zoo {

class Application {
public:
    struct Settings {
        Settings(int argc, char** argv) noexcept;
    };

    struct Version {
        uint16_t major_;
        uint16_t minor_;
        uint16_t patch_;
    };

    struct Context {
        Version version_;
    };

    enum ExitStatus { Ok, Err };

private:
    Settings settings_;
};

} // namespace zoo
