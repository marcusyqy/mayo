#pragma once

namespace zoo {

class Application {
public:
    struct Settings {
        Settings(int argc, char** argv) noexcept;
    };
    enum ExitStatus { Ok, Err };

private:
    Settings settings_;
};

} // namespace zoo
