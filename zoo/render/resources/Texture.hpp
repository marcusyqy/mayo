#pragma once
#include "render/fwd.hpp"

#include "Allocator.hpp"
#include <stdx/function_ref.hpp>

namespace zoo::render::resources {

class Texture;

class TextureView {
public:
    TextureView(
        const Texture& reference, VkImageViewCreateInfo create_info) noexcept;
    ~TextureView() noexcept;

    TextureView(const TextureView&) = delete;
    TextureView& operator=(const TextureView&) = delete;

private:
    const Texture& reference_;
    VkImageViewCreateInfo create_info_;
    VkImageView view_;
};

namespace texture {

class Builder {
public:
    Builder(const Allocator& allocator, std::string_view name) noexcept;

    Texture build() noexcept;

private:
    std::string name_;

    VkDevice device_;
    VmaAllocator allocator_ = {};
    VmaAllocation allocation_ = {};
    VmaAllocationInfo allocation_info_ = {};
    VmaMemoryUsage memory_usage_ = VMA_MEMORY_USAGE_AUTO;

    VkImageType image_type_ = VK_IMAGE_TYPE_2D;
    VkFormat format_ = VK_FORMAT_UNDEFINED; // Needs to be set.
    VkExtent3D extent_ = {};

    VkSampleCountFlags samples_ = VK_SAMPLE_COUNT_1_BIT;
    VkImageTiling tiling_ = VK_IMAGE_TILING_OPTIMAL;

    uint32_t mip_level_ = 1;
    uint32_t arr_level_ = 1;

    VkImageUsageFlags usage_flags_ = {};
};

} // namespace texture

class Texture {
public:
    using builder_type = texture::Builder;

    static builder_type start_build(
        const Allocator& allocator, std::string_view name) noexcept;

    explicit Texture(std::string name, VkImage image,
        VkImageCreateInfo create_info, VkDevice device, VmaAllocator allocator,
        VmaAllocation allocation, VmaAllocationInfo allocation_info) noexcept;

    operator TextureView&() noexcept;
    operator const TextureView&() const noexcept;

    std::string_view name() const noexcept { return name_; }

    VkDevice device() const noexcept { return device_; }

private:
    VkImageViewCreateInfo create_image_view_info() const noexcept;

private:
    std::string name_;

    VkImage image_;
    VkImageCreateInfo create_info_ = {};

    VkDevice device_;
    VmaAllocator allocator_;
    VmaAllocation allocation_;
    VmaAllocationInfo allocation_info_;

    TextureView view_;
};

} // namespace zoo::render::resources
