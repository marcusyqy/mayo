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
        // TODO: add stencil
        aspect_mask |= VK_IMAGE_ASPECT_DEPTH_BIT;
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

Builder& Builder::allocation_type(VmaMemoryUsage usage) noexcept {
    memory_usage_ = usage;
    return *this;
}

Builder& Builder::allocation_required_flags(
    VkMemoryPropertyFlags flags) noexcept {
    memory_properties_flags_ = flags;
    return *this;
}

Builder& Builder::mip(uint32_t level) noexcept {
    mip_level_ = level;
    return *this;
}
Builder& Builder::array(uint32_t level) noexcept {
    arr_level_ = level;
    return *this;
}

Builder& Builder::format(VkFormat format) noexcept {
    format_ = format;
    return *this;
}

Builder& Builder::usage(VkImageUsageFlags usage) noexcept {
    usage_flags_ = usage;
    return *this;
}
Builder& Builder::type(VkImageType type) noexcept {
    image_type_ = type;
    return *this;
}

Builder& Builder::samples(VkSampleCountFlagBits count) noexcept {
    samples_ = count;
    return *this;
}

Builder& Builder::tiling(VkImageTiling tile) noexcept {
    tiling_ = tile;
    return *this;
}

Builder& Builder::extent(VkExtent3D extent) noexcept {
    extent_ = extent;
    return *this;
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
    info.usage = usage_flags_;

    // TODO: set this as some variable as well.
    info.samples = samples_;
    info.tiling = tiling_;

    VmaAllocationCreateInfo alloc_info{};
    alloc_info.usage = memory_usage_;
    alloc_info.requiredFlags = memory_properties_flags_;

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

Texture::Texture(Texture&& other) noexcept
    : name_(std::move(other.name_)), image_(std::move(other.image_)),
      create_info_(std::move(other.create_info_)),
      device_(std::move(other.device_)),
      allocator_(std::move(other.allocator_)),
      allocation_(std::move(other.allocation_)),
      allocation_info_(std::move(other.allocation_info_)),
      view_(std::move(other.view_)) {
    view_.update(*this);
    other.invalidate();
}

Texture& Texture::operator=(Texture&& other) noexcept {
    destroy();
    name_ = std::move(other.name_);
    image_ = std::move(other.image_);
    create_info_ = std::move(other.create_info_);
    device_ = std::move(other.device_);
    allocator_ = std::move(other.allocator_);
    allocation_ = std::move(other.allocation_);
    allocation_info_ = std::move(other.allocation_info_);
    view_ = std::move(other.view_);
    view_.update(*this);
    other.invalidate();
    return *this;
}

Texture::~Texture() noexcept { destroy(); }

void Texture::destroy() noexcept {
    view_.destroy();
    if (image_ != nullptr) {
        vmaDestroyImage(allocator_, image_, allocation_);
        image_ = nullptr;
    }
}

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
Texture::builder_type Texture::start_build(
    const Allocator& allocator, std::string_view name) noexcept {
    return {allocator, name};
}

TextureView& Texture::view() noexcept { return view_; }
const TextureView& Texture::view() const noexcept { return view_; }

bool Texture::valid() const noexcept { return image_ != nullptr; }
void Texture::invalidate() noexcept { image_ = nullptr; }

TextureView::TextureView(
    const Texture& reference, VkImageViewCreateInfo create_info) noexcept
    : reference_(&reference), create_info_(create_info), view_(nullptr) {
    VK_EXPECT_SUCCESS(
        vkCreateImageView(reference.device(), &create_info_, nullptr, &view_));
}

void TextureView::destroy() noexcept {
    if (view_ != nullptr && reference_->valid()) {
        vkDestroyImageView(reference_->device(), view_, nullptr);
        view_ = nullptr;
    }
}

TextureView::~TextureView() noexcept { destroy(); }

TextureView::TextureView(TextureView&& other) noexcept
    : reference_(other.reference_), create_info_(std::move(other.create_info_)),
      view_(std::move(other.view_)) {
    other.invalidate();
}

TextureView& TextureView::operator=(TextureView&& other) noexcept {
    // don't set this to null because we don't want a `nullptr`
    destroy();
    reference_ = other.reference_;
    create_info_ = std::move(other.create_info_);
    view_ = std::move(other.view_);
    other.invalidate();
    return *this;
}

void TextureView::invalidate() noexcept { view_ = nullptr; }
bool TextureView::valid() const noexcept { return view_ != nullptr; }

// feels like a hack...
void TextureView::update(Texture& reference) noexcept {
    reference_ = &reference;
}

} // namespace zoo::render::resources
