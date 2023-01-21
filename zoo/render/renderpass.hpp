#pragma once

#include "device_context.hpp"
#include "fwd.hpp"
#include "utils/box.hpp"

namespace zoo::render {

class renderpass : public utils::box<VkRenderPass> {
public:
    using underlying_type = utils::box<VkRenderPass>;

    renderpass() noexcept;
    renderpass(
        std::shared_ptr<device_context> context, VkFormat format) noexcept;
    ~renderpass() noexcept;

    renderpass& operator=(renderpass&& renderpass) noexcept;
    renderpass(renderpass&& renderpass) noexcept;

    using underlying_type::get;
    using underlying_type::operator underlying_type::value_type;
    using underlying_type::emplace;
};

} // namespace zoo::render
