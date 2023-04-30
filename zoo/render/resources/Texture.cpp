#include "Texture.hpp"

namespace zoo::render::resources {

namespace {

VkImageViewType vk_image_type_to_image_view_type(
    VkImageType image_type) noexcept {
    switch (image_type) {
    case VK_IMAGE_TYPE_1D:
        return VK_IMAGE_VIEW_TYPE_1D;
    case VK_IMAGE_TYPE_2D:
        return VK_IMAGE_VIEW_TYPE_2D;
    case VK_IMAGE_TYPE_3D:
        return VK_IMAGE_VIEW_TYPE_3D;
    default:
        ZOO_LOG_ERROR(
            "Something went wrong with converting image to image view type!");
        return VK_IMAGE_VIEW_TYPE_MAX_ENUM;
    }
}

VkImageAspectFlags vk_image_usage_to_aspect_mask(
    VkImageUsageFlags usage_flags) noexcept {
    VkImageAspectFlags aspect_mask = VK_IMAGE_ASPECT_NONE;

    if (usage_flags & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) {
        aspect_mask |= VK_IMAGE_ASPECT_COLOR_BIT;
    }

    if (usage_flags & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) {
        aspect_mask |=
            (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);
    }

    return aspect_mask;
}
} // namespace
namespace texture {

Builder::Builder(const Allocator& allocator, std::string_view name) noexcept {
    allocator_ = allocator;
    device_ = allocator.device();
    name_ = name;
}

Texture Builder::build() noexcept {

    VkImageCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    info.pNext = nullptr;
    info.imageType = image_type_;

    if (format_ == VK_FORMAT_UNDEFINED) {
        ZOO_LOG_ERROR("Format created for texture is undefined, {}", name_);
    }

    info.format = format_;
    info.extent = extent_;

    info.mipLevels = mip_level_;
    info.arrayLayers = arr_level_;
    info.samples = VK_SAMPLE_COUNT_1_BIT;
    info.tiling = VK_IMAGE_TILING_OPTIMAL;
    info.usage = usage_flags_;

    VmaAllocationCreateInfo alloc_info{};
    alloc_info.usage = memory_usage_;

    VkImage image = {};
    VK_EXPECT_SUCCESS(vmaCreateImage(allocator_, &info, &alloc_info, &image,
        &allocation_, &allocation_info_));

    return Texture{
        name_, image, info, device_, allocator_, allocation_, allocation_info_};
}

} // namespace texture

Texture::Texture(std::string name, VkImage image, VkImageCreateInfo create_info,
    VkDevice device, VmaAllocator allocator, VmaAllocation allocation,
    VmaAllocationInfo allocation_info) noexcept
    : name_(name), image_(image), create_info_(create_info), device_(device),
      allocator_(allocator), allocation_(allocation),
      allocation_info_(allocation_info),
      view_(*this, create_image_view_info()) {}

VkImageViewCreateInfo Texture::create_image_view_info() const noexcept {
    VkImageViewCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    info.pNext = nullptr;
    info.viewType = vk_image_type_to_image_view_type(create_info_.imageType);
    info.image = image_;
    info.format = create_info_.format;
    info.subresourceRange.baseMipLevel = 0;
    info.subresourceRange.levelCount = create_info_.mipLevels;
    info.subresourceRange.baseArrayLayer = 0;
    info.subresourceRange.layerCount = create_info_.arrayLayers;

    // TODO: determine if we should have a seperate aspect mask field when
    // creating.
    info.subresourceRange.aspectMask =
        vk_image_usage_to_aspect_mask(create_info_.usage);

    return info;
}

Texture::operator TextureView&() noexcept { return view_; }
Texture::operator const TextureView&() const noexcept { return view_; }

TextureView::TextureView(
    const Texture& reference, VkImageViewCreateInfo create_info) noexcept
    : reference_(reference), create_info_(create_info), view_(nullptr) {
    VK_EXPECT_SUCCESS(
        vkCreateImageView(reference.device(), &create_info_, nullptr, &view_));
}

TextureView::~TextureView() noexcept {
    if (view_ != nullptr) {
        vkDestroyImageView(reference_.device(), view_, nullptr);
    }
}

} // namespace zoo::render::resources
