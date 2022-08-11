#pragma once

namespace zoo {

class application {
public:
    struct settings {
        settings(int argc, char** argv) noexcept;
    };
    enum exit_status { ok, err };
};

} // namespace zoo
