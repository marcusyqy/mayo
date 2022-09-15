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

    [[nodiscard]] auto get_info() noexcept -> info;
    [[nodiscard]] auto get_extensions() noexcept -> std::vector<const char*>;
    [[nodiscard]] auto get_layers() noexcept -> std::vector<const char*>;

private:
    parameters parameters_;
};
} // namespace vulkan
} // namespace zoo::platform
