#pragma once
#include "core/fwd.hpp"
#include <cstdint>
#include <string>

namespace zoo::application_detail {

struct settings {
    settings(int num_args, char** args) noexcept;
};

using version = core::version;

struct info {
    version version;
    std::string name;
};

enum exit_status { ok, err };

} // namespace zoo::application
