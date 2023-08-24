#pragma once
#include "type.hpp"
#include <memory>

namespace refl::meta {

struct object_node {
    std::shared_ptr<void> value_;
};

struct function_node {
    std::shared_ptr<void> param_;
    void (*func_)(void*) noexcept;
};

struct type_node {};

struct class_node {};

} // namespace refl::meta
