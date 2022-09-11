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

    [[nodiscard]] Info get_info() noexcept;
    [[nodiscard]] std::vector<const char*> get_extensions() noexcept;
    [[nodiscard]] std::vector<const char*> get_layers() noexcept;

private:
    Parameters parameters_;
};
} // namespace vulkan
} // namespace zoo::platform