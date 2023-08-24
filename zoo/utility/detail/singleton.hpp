#pragma once

namespace zoo::utils {

template <typename T>
struct Singleton {
    static T& instance() noexcept {
        static T _instance;
        return _instance;
    }
};

} // namespace zoo::detail
