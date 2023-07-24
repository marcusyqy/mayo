#pragma once
#include "core/fwd.hpp"
#include "render/fwd.hpp"

#include "Allocator.hpp"
#include <stdx/function_ref.hpp>

namespace zoo::render::resources {

class Texture;

/// This may break if another device appears.
class TextureView {
public:
    TextureView(const Texture& reference, VkImageViewCreateInfo create_info) noexcept;

    TextureView() = default;
    ~TextureView() noexcept;

    void destroy() noexcept;

    TextureView(const TextureView&)            = delete;
    TextureView& operator=(const TextureView&) = delete;

    TextureView(TextureView&& other) noexcept;
    TextureView& operator=(TextureView&& other) noexcept;

    void invalidate() noexcept;
    bool valid() const noexcept;

    operator VkImageView() const noexcept { return view_; }

private:
    std::string_view name_ = "ZOO_UNINITIALIZED_TEXTURE_VIEW";
    VkDevice device_       = VK_NULL_HANDLE;

    VkImageViewCreateInfo create_info_ = {};
    VkImageView view_                  = VK_NULL_HANDLE;
};

namespace texture {

class Builder {
public:
    Builder(std::string_view name) noexcept;

    Texture build(const Allocator& allocator) noexcept;

    Builder& allocation_type(VmaMemoryUsage usage) noexcept;
    Builder& allocation_required_flags(VkMemoryPropertyFlags flags) noexcept;

    Builder& mip(uint32_t level) noexcept;
    Builder& array(uint32_t level) noexcept;
    Builder& format(VkFormat format) noexcept;

    Builder& usage(VkImageUsageFlags usage) noexcept;
    Builder& type(VkImageType type) noexcept;
    Builder& samples(VkSampleCountFlagBits count) noexcept;
    Builder& tiling(VkImageTiling tile) noexcept;
    Builder& extent(VkExtent3D extent) noexcept;

private:
    std::string name_;

    VmaAllocationInfo allocation_info_ = {};
    VmaMemoryUsage memory_usage_       = VMA_MEMORY_USAGE_AUTO;

    VkImageType image_type_ = VK_IMAGE_TYPE_2D;
    VkFormat format_        = VK_FORMAT_UNDEFINED; // Needs to be set.
    VkExtent3D extent_      = {};

    VkSampleCountFlagBits samples_ = VK_SAMPLE_COUNT_1_BIT;
    VkImageTiling tiling_          = VK_IMAGE_TILING_OPTIMAL;

    uint32_t mip_level_ = 1;
    uint32_t arr_level_ = 1;

    VkMemoryPropertyFlags memory_properties_flags_ = {};
    VkImageUsageFlags usage_flags_                 = {};
};

} // namespace texture

class Texture {
public:
    using builder_type = texture::Builder;

    static builder_type start_build(std::string_view name) noexcept;

    explicit Texture(
        std::string name,
        VkImage image,
        VkImageCreateInfo create_info,
        VkDevice device,
        VmaAllocator allocator,
        VmaAllocation allocation,
        VmaAllocationInfo allocation_info) noexcept;

    Texture() noexcept = default;

    ~Texture() noexcept;

    Texture(const Texture& other) noexcept            = delete;
    Texture& operator=(const Texture& other) noexcept = delete;

    Texture(Texture&& other) noexcept;
    Texture& operator=(Texture&& other) noexcept;

    operator TextureView&() noexcept { return view(); }
    operator const TextureView&() const noexcept { return view(); }

    TextureView& view() noexcept;
    const TextureView& view() const noexcept;

    std::string_view name() const noexcept { return name_; }

    VkDevice device() const noexcept { return device_; }

    void invalidate() noexcept;
    bool valid() const noexcept;

    void destroy() noexcept;

    VkImageLayout layout() const noexcept;
    void layout(VkImageLayout layout) noexcept;

    size_t allocated_size() const noexcept;

    u32 mip_level() const noexcept;
    u32 array_count() const noexcept;
    VkExtent3D extent() const noexcept;

    VkImage handle() const noexcept;

private:
    VkImageViewCreateInfo create_image_view_info() const noexcept;

private:
    std::string name_ = "ZOO_UNINITIALIZED_TEXTURE";

    VkImage image_                 = VK_NULL_HANDLE;
    VkImageCreateInfo create_info_ = {};

    VkDevice device_                   = VK_NULL_HANDLE;
    VmaAllocator allocator_            = VK_NULL_HANDLE;
    VmaAllocation allocation_          = VK_NULL_HANDLE;
    VmaAllocationInfo allocation_info_ = {};

    TextureView view_;
};

} // namespace zoo::render::resources
