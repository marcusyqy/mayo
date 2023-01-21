#pragma once

#include "render/fwd.hpp"
#include "render/utils/box.hpp"

namespace zoo::render::sync {
class semaphore : public utils::box<VkSemaphore> {
public:
    using underlying_type = utils::box<VkSemaphore>;
    semaphore(std::shared_ptr<device_context> context) noexcept;

    using underlying_type::get;
    using underlying_type::operator underlying_type::value_type;

private:
};
} // namespace zoo::render::sync
