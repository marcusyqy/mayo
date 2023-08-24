#pragma once
#include <tuple>

namespace zoo::utils {

template <typename... Ts>
class Initializer {
    using FunctionPtr = void (*)(Ts&&... args);
    void invoke(FunctionPtr ptr) noexcept {
        if (ptr != nullptr) std::apply(ptr, data_);
    }

    void reset() noexcept {
        on_construct_ = nullptr;
        on_destruct_  = nullptr;
        data_         = {};
    }

public:
    Initializer(FunctionPtr on_construct, FunctionPtr on_destruct, Ts&&... datas) noexcept :
        on_construct_(on_construct), on_destruct_(on_destruct), data_(std::make_tuple(std::forward<Ts&&>(datas)...)) {
        invoke(on_construct_);
    }
    ~Initializer() noexcept { invoke(on_destruct_); }

    Initializer(const Initializer& o)            = delete;
    Initializer& operator=(const Initializer& o) = delete;

    Initializer(Initializer&& o) noexcept :
        on_construct_(o.on_construct_), on_destruct_(o.on_destruct_), data_(std::move(o.data_)) {
        o.reset();
    }

    Initializer& operator=(Initializer&& o) noexcept {
        invoke(on_destruct_);
        on_construct_ = o.on_construct_;
        on_destruct_  = o.on_destruct_;
        data_         = std::move(o.data_);
        o.reset();
        return *this;
    }

private:
    FunctionPtr on_construct_{ nullptr };
    FunctionPtr on_destruct_{ nullptr };
    std::tuple<Ts...> data_{};
};

} // namespace zoo::core
