#pragma once

#include "render/fwd.hpp"
#include "render/utils/Box.hpp"

namespace zoo::render::sync {

class Fence : public utils::Box<VkFence> {
public:
    using underlying_type = utils::Box<VkFence>;
    Fence(std::shared_ptr<DeviceContext> context) noexcept;

    void reset() noexcept;
    void wait() noexcept;

    using underlying_type::get;
    using underlying_type::operator underlying_type::value_type;

private:
};

} // namespace zoo::render::sync
