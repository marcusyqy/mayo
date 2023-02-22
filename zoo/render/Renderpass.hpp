#pragma once

#include "DeviceContext.hpp"
#include "fwd.hpp"
#include "utils/box.hpp"

namespace zoo::render {

class Renderpass : public utils::Box<VkRenderPass> {
public:
    using underlying_type = utils::Box<VkRenderPass>;

    Renderpass() noexcept;
    Renderpass(
        std::shared_ptr<DeviceContext> context, VkFormat format) noexcept;
    ~Renderpass() noexcept = default;

    Renderpass& operator=(Renderpass&& renderpass) noexcept;
    Renderpass(Renderpass&& renderpass) noexcept;

    Renderpass& operator=(const Renderpass&& renderpass) noexcept = delete;
    Renderpass(const Renderpass&& renderpass) noexcept = delete;

    using underlying_type::get;
    using underlying_type::operator underlying_type::value_type;
    using underlying_type::emplace;
};

} // namespace zoo::render
