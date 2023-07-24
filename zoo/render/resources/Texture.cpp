#include "Texture.hpp"
#include "core/fwd.hpp"

namespace zoo::render::resources {

namespace {

VkImageViewType vk_image_type_to_image_view_type(VkImageType image_type) noexcept {
    switch (image_type) {
        case VK_IMAGE_TYPE_1D: return VK_IMAGE_VIEW_TYPE_1D;
        case VK_IMAGE_TYPE_2D: return VK_IMAGE_VIEW_TYPE_2D;
        case VK_IMAGE_TYPE_3D: return VK_IMAGE_VIEW_TYPE_3D;
        default:
            ZOO_LOG_ERROR("Something went wrong with converting image to image "
                          "view type!");
            return VK_IMAGE_VIEW_TYPE_MAX_ENUM;
    }
}

VkImageAspectFlags vk_image_usage_to_aspect_mask(VkImageUsageFlags usage_flags) noexcept {
    VkImageAspectFlags aspect_mask = VK_IMAGE_ASPECT_NONE;
    if (usage_flags & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT || usage_flags & VK_IMAGE_USAGE_SAMPLED_BIT) {
        aspect_mask |= VK_IMAGE_ASPECT_COLOR_BIT;
    }
    if (usage_flags & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) {
        // TODO: add stencil
        aspect_mask |= VK_IMAGE_ASPECT_DEPTH_BIT;
    }

    ZOO_ASSERT(aspect_mask != VK_IMAGE_ASPECT_NONE);
    return aspect_mask;
}

size_t get_format_size(VkFormat format) noexcept {
    ZOO_ASSERT(format == VK_FORMAT_R8G8B8A8_SRGB);
    return 4;
}

} // namespace

namespace texture {

Builder::Builder(std::string_view name) noexcept { name_ = name; }

Builder& Builder::allocation_type(VmaMemoryUsage usage) noexcept {
    memory_usage_ = usage;
    return *this;
}

Builder& Builder::allocation_required_flags(VkMemoryPropertyFlags flags) noexcept {
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

Texture Builder::build(const Allocator& allocator) noexcept {
    VkImageCreateInfo info{};
    info.sType     = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    info.pNext     = nullptr;
    info.imageType = image_type_;

    if (format_ == VK_FORMAT_UNDEFINED) {
        ZOO_LOG_ERROR("Format created for texture is undefined, {}", name_);
    }

    info.format      = format_;
    info.extent      = extent_;
    info.mipLevels   = mip_level_;
    info.arrayLayers = arr_level_;
    info.usage       = usage_flags_;

    // TODO: set this as some variable as well.
    info.samples = samples_;
    info.tiling  = tiling_;

    VmaAllocationCreateInfo alloc_info{};
    alloc_info.usage         = memory_usage_;
    alloc_info.requiredFlags = memory_properties_flags_;

    VkImage image            = {};
    VmaAllocation allocation = {};
    VK_EXPECT_SUCCESS(vmaCreateImage(allocator, &info, &alloc_info, &image, &allocation, &allocation_info_));

    return Texture{ name_, image, info, allocator.device(), allocator, allocation, allocation_info_ };
}

} // namespace texture

Texture::Texture(
    std::string name,
    VkImage image,
    VkImageCreateInfo create_info,
    VkDevice device,
    VmaAllocator allocator,
    VmaAllocation allocation,
    VmaAllocationInfo allocation_info) noexcept :
    name_(name),
    image_(image), create_info_(create_info), device_(device), allocator_(allocator), allocation_(allocation),
    allocation_info_(allocation_info), view_(*this, create_image_view_info()) {}

Texture::Texture(Texture&& other) noexcept :
    name_(std::move(other.name_)), image_(std::move(other.image_)), create_info_(std::move(other.create_info_)),
    device_(std::move(other.device_)), allocator_(std::move(other.allocator_)),
    allocation_(std::move(other.allocation_)), allocation_info_(std::move(other.allocation_info_)),
    view_(std::move(other.view_)) {
    other.invalidate();
}

Texture& Texture::operator=(Texture&& other) noexcept {
    destroy();
    name_            = std::move(other.name_);
    image_           = std::move(other.image_);
    create_info_     = std::move(other.create_info_);
    device_          = std::move(other.device_);
    allocator_       = std::move(other.allocator_);
    allocation_      = std::move(other.allocation_);
    allocation_info_ = std::move(other.allocation_info_);
    view_            = std::move(other.view_);
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
    info.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    info.pNext                           = nullptr;
    info.viewType                        = vk_image_type_to_image_view_type(create_info_.imageType);
    info.image                           = image_;
    info.format                          = create_info_.format;
    info.subresourceRange.baseMipLevel   = 0;
    info.subresourceRange.levelCount     = create_info_.mipLevels;
    info.subresourceRange.baseArrayLayer = 0;
    info.subresourceRange.layerCount     = create_info_.arrayLayers;

    // TODO: determine if we should have a seperate aspect mask field when
    // creating.
    info.subresourceRange.aspectMask = vk_image_usage_to_aspect_mask(create_info_.usage);

    return info;
}
Texture::builder_type Texture::start_build(std::string_view name) noexcept { return { name }; }

VkImageLayout Texture::layout() const noexcept { return create_info_.initialLayout; }
void Texture::layout(VkImageLayout layout) noexcept { create_info_.initialLayout = layout; }
VkAccessFlags Texture::access_flags() const noexcept { return access_flags_; }
void Texture::access_flags(VkAccessFlags flags) noexcept { access_flags_ = flags; }

VkImage Texture::handle() const noexcept { return image_; }

u32 Texture::mip_level() const noexcept { return create_info_.mipLevels; }
u32 Texture::array_count() const noexcept { return create_info_.arrayLayers; }

VkExtent3D Texture::extent() const noexcept { return create_info_.extent; }

size_t Texture::allocated_size() const noexcept {
    return create_info_.extent.width * create_info_.extent.height * create_info_.extent.depth *
        get_format_size(create_info_.format);
}

TextureView& Texture::view() noexcept { return view_; }
const TextureView& Texture::view() const noexcept { return view_; }

bool Texture::valid() const noexcept { return image_ != nullptr; }
void Texture::invalidate() noexcept { image_ = nullptr; }

TextureView::TextureView(const Texture& reference, VkImageViewCreateInfo create_info) noexcept :
    name_(reference.name()), device_(reference.device()), create_info_(create_info), view_(nullptr) {
    VK_EXPECT_SUCCESS(vkCreateImageView(reference.device(), &create_info_, nullptr, &view_));
}

void TextureView::destroy() noexcept {
    if (view_ != nullptr) {
        vkDestroyImageView(device_, view_, nullptr);
        view_ = nullptr;
    }
}

TextureView::~TextureView() noexcept { destroy(); }

TextureView::TextureView(TextureView&& other) noexcept :
    name_(std::move(other.name_)), device_(std::move(other.device_)), create_info_(std::move(other.create_info_)),
    view_(std::move(other.view_)) {
    other.invalidate();
}

TextureView& TextureView::operator=(TextureView&& other) noexcept {
    // don't set this to null because we don't want a `nullptr`
    destroy();
    name_ = std::move(other.name_);

    // TODO: will this break if another device comes to play?
    device_      = std::move(other.device_);
    create_info_ = std::move(other.create_info_);
    view_        = std::move(other.view_);
    other.invalidate();
    return *this;
}

void TextureView::invalidate() noexcept { view_ = nullptr; }
bool TextureView::valid() const noexcept { return view_ != nullptr; }

namespace texture_sampler {

TextureSampler Builder::build(DeviceContext& context) noexcept {
    VkSamplerCreateInfo info = {
        .sType        = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .pNext        = nullptr,
        .magFilter    = mag_filter_,
        .minFilter    = min_filter_,
        .addressModeU = address_mode_.u,
        .addressModeV = address_mode_.v,
        .addressModeW = address_mode_.w,
    };

    VkSampler sampler;
    vkCreateSampler(context, &info, nullptr, &sampler);

    return {
        context,
        sampler,
        info // FOR DEBUG
    };
}

Builder& Builder::address_mode(VkSamplerAddressMode address) noexcept {
    return address_mode_u(address).address_mode_v(address).address_mode_w(address);
}

// @Evaluate : looks very prone to error.
Builder& Builder::address_mode_u(VkSamplerAddressMode address) noexcept {
    address_mode_.u = address;
    return *this;
}

Builder& Builder::address_mode_v(VkSamplerAddressMode address) noexcept {
    address_mode_.v = address;
    return *this;
}
Builder& Builder::address_mode_w(VkSamplerAddressMode address) noexcept {
    address_mode_.w = address;
    return *this;
}

Builder& Builder::min_filter(VkFilter filter) noexcept {
    min_filter_ = filter;
    return *this;
}
Builder& Builder::mag_filter(VkFilter filter) noexcept {
    mag_filter_ = filter;
    return *this;
}

}; // namespace texture_sampler

TextureSampler::TextureSampler(VkDevice device, VkSampler sampler, VkSamplerCreateInfo create_info) noexcept :
    device_(device), sampler_(sampler), create_info_(create_info) {}

TextureSampler::TextureSampler(TextureSampler&& o) noexcept { *this = std::move(o); }

TextureSampler& TextureSampler::operator=(TextureSampler&& o) noexcept {
    if (device_ && sampler_) {
        vkDestroySampler(device_, sampler_, nullptr);
    }

    device_      = o.device_;
    sampler_     = o.sampler_;
    create_info_ = o.create_info_;

    o.device_      = nullptr;
    o.sampler_     = nullptr;
    o.create_info_ = {};

    return *this;
}
TextureSampler::~TextureSampler() noexcept {
    if (device_ && sampler_) {
        vkDestroySampler(device_, sampler_, nullptr);
    }
}

TextureSampler::builder_type TextureSampler::start_build() noexcept { return {}; }

} // namespace zoo::render::resources
