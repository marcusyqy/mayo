#pragma once

#include "render/fwd.hpp"
#include "render/utils/box.hpp"

namespace zoo::render::sync {

class fence : public utils::box<VkFence> {
public:
    using underlying_type = utils::box<VkFence>;
    fence(std::shared_ptr<device_context> context) noexcept;

    void reset() noexcept;
    void wait() noexcept;

    using underlying_type::get;
    using underlying_type::operator underlying_type::value_type;

private:
};

} // namespace zoo::render::sync
