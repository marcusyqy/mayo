#pragma once

#include "render/fwd.hpp"
#include "render/utils/Box.hpp"

namespace zoo::render::sync {
class Semaphore : public utils::Box<VkSemaphore> {
public:
    using underlying_type = utils::Box<VkSemaphore>;
    Semaphore(std::shared_ptr<DeviceContext> context) noexcept;

    using underlying_type::get;
    using underlying_type::operator underlying_type::value_type;

private:
};
} // namespace zoo::render::sync
