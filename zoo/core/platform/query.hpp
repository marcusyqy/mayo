#pragma once
#include <vector>

namespace zoo::platform {

namespace vulkan {
struct Info {
    std::vector<const char*> extensions_;
    std::vector<const char*> layers_;
};

struct Parameters {
    bool validation_;
};

class Query {
public:
    Query(Parameters parameters) noexcept;

    [[nodiscard]] auto get_info() noexcept -> Info;
    [[nodiscard]] auto get_extensions() noexcept -> std::vector<const char*>;
    [[nodiscard]] auto get_layers() noexcept -> std::vector<const char*>;

private:
    Parameters parameters_;
};
} // namespace vulkan
} // namespace zoo::platform