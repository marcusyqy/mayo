#pragma once
#include <vector>

namespace zoo::platform {

namespace vulkan {
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

    [[nodiscard]] info get_info() noexcept;
    [[nodiscard]] std::vector<const char*> get_extensions() noexcept;
    [[nodiscard]] std::vector<const char*> get_layers() noexcept;

private:
    parameters parameters_;
};
} // namespace vulkan
} // namespace zoo::platform
