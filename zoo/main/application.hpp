#pragma once
#include "info.hpp"

namespace zoo {

class application {
public:
    using settings = application_detail::settings;
    using exit_status = application_detail::exit_status;
    using info = application_detail::info;

private:
    settings settings_;
};

} // namespace zoo
