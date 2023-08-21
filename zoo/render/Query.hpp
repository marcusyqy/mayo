#pragma once
#include <vector>

namespace zoo::render {

struct Info {
    std::vector<const char*> extensions;
    std::vector<const char*> layers;
};

struct Parameters {
    bool validation;
};

class Query {
public:
    Query(Parameters parameters) noexcept;

    [[nodiscard]] Info get_info() const noexcept;
    [[nodiscard]] std::vector<const char*> get_extensions() const noexcept;
    [[nodiscard]] std::vector<const char*> get_layers() const noexcept;
    [[nodiscard]] const Parameters& get_params() const noexcept;

private:
    Parameters parameters_;
};

} // namespace zoo::platform::render
