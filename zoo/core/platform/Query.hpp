#pragma once
#include <vector>

namespace zoo::platform::render {

struct info {
    std::vector<const char*> extensions_;
    std::vector<const char*> layers_;
};

struct Parameters {
    bool validation_;
};

class Query {
public:
    Query(Parameters parameters) noexcept;

    [[nodiscard]] info get_info() const noexcept;
    [[nodiscard]] std::vector<const char*> get_extensions() const noexcept;
    [[nodiscard]] std::vector<const char*> get_layers() const noexcept;
    [[nodiscard]] const Parameters& get_params() const noexcept;

private:
    Parameters parameters_;
};

} // namespace zoo::platform::render
