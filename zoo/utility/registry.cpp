#include "registry.hpp"

namespace zoo {

Registry::handle Registry::create() noexcept {
    if (start_ == invalid) {
        auto next = sparse_.size();
        sparse_.push_back(next);
        return next;
    }

    ZOO_ASSERT(start_ < sparse_.size());
    auto next     = start_;
    start_        = sparse_[start_];
    sparse_[next] = invalid;
    return next;
}

void Registry::destroy(handle idx) noexcept {
    ZOO_ASSERT(idx != invalid && idx < sparse_.size());
    sparse_[idx] = start_;
    start_       = idx;
}

bool Registry::all_freed() const noexcept {
    index_type i = 0;
    for (const auto& s : sparse_) {
        if (s == invalid && i++ != 0) return false; // only one can be invalid ( which is tail )
    }
    return true;
}

} // namespace zoo
