#pragma once
#include "core/fwd.hpp"
#include <limits>
#include <vector>

namespace zoo {

class Registry {
public:
    using size_type  = size_t;
    using index_type = size_type;
    using handle     = index_type;

    handle create() noexcept;
    void destroy(handle idx) noexcept;

    [[nodiscard]] bool all_freed() const noexcept;

    static constexpr auto invalid = std::numeric_limits<handle>::max();

private:
    std::vector<handle> sparse_ = {};
    handle start_               = invalid;
};

} // namespace zoo
