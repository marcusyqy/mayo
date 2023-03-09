#pragma once

#include "DeviceContext.hpp"
#include "fwd.hpp"
#include "utils/box.hpp"

namespace zoo::render {

class RenderPass : public utils::Box<VkRenderPass> {
public:
    using underlying_type = VkRenderPass; // utils::Box<VkRenderPass>;

    RenderPass() noexcept;
    RenderPass(
        DeviceContext& context, VkFormat format) noexcept;

    ~RenderPass() noexcept;

    RenderPass& operator=(RenderPass&& renderpass) noexcept;
    RenderPass(RenderPass&& renderpass) noexcept;

    RenderPass& operator=(const RenderPass&& renderpass) noexcept = delete;
    RenderPass(const RenderPass&& renderpass) noexcept = delete;


    void emplace(DeviceContext& device, underlying_type type); {
        context_ = std::addressof(device);
        type_ = type;
    }
    using underlying_type::get;
    using underlying_type::operator underlying_type::value_type;
    using underlying_type::emplace;
};

} // namespace zoo::render
