#pragma once

namespace zoo::utils {

// This serves as a way to generate a singleton variable if we do not have this already.
template <typename T>
struct Singleton {
    static T& instance() noexcept {
        static T _instance;
        return _instance;
    }
};

} // namespace zoo::utils
