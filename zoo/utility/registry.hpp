#pragma once
#include "core/fwd.hpp"
#include <vector>

namespace zoo {

// @BOOKMARK
// @TODO: optimize.
class Registry {
public:
    s32 create() noexcept;
    void destroy(s32 idx) noexcept;

private:
    std::vector<s32> sparse_ = {};
    s32 start_               = -1;
};

} // namespace zoo
