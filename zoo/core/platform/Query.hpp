#pragma once
#include <vector>

namespace zoo::platform::render {

struct info {
    std::vector<const char*> extensions_;
    std::vector<const char*> layers_;
};

struct parameters {
    bool validation_;
};

class query {
public:
    query(parameters parameters) noexcept;

    [[nodiscard]] info get_info() const noexcept;
    [[nodiscard]] std::vector<const char*> get_extensions() const noexcept;
    [[nodiscard]] std::vector<const char*> get_layers() const noexcept;
    [[nodiscard]] const parameters& get_params() const noexcept;

private:
    parameters parameters_;
};

} // namespace zoo::platform::render
