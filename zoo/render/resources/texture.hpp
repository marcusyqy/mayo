#pragma once
#include "core/fwd.hpp"
#include "render/fwd.hpp"

#include "allocator.hpp"
#include "render/device_context.hpp"
#include <stdx/function_ref.hpp>

namespace zoo::render::resources {

class Texture;

/// This may break if another device appears.
class TextureView {
public:
    TextureView(const Texture& reference, VkImageViewCreateInfo create_info) noexcept;
    TextureView(std::string name, VkDevice device, VkImageViewCreateInfo create_info) noexcept;

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
    VkAccessFlags access_flags() const noexcept;
    void access_flags(VkAccessFlags flags) noexcept;

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
    VkAccessFlags access_flags_    = {};

    VkDevice device_                   = VK_NULL_HANDLE;
    VmaAllocator allocator_            = VK_NULL_HANDLE;
    VmaAllocation allocation_          = VK_NULL_HANDLE;
    VmaAllocationInfo allocation_info_ = {};

    TextureView view_;
};

class TextureSampler;

namespace texture_sampler {

struct Builder {

    TextureSampler build(Device_Context& context) noexcept;

    Builder& address_mode(VkSamplerAddressMode address) noexcept;

    // @Evaluate : looks very prone to error.
    Builder& address_mode_u(VkSamplerAddressMode address) noexcept;
    Builder& address_mode_v(VkSamplerAddressMode address) noexcept;
    Builder& address_mode_w(VkSamplerAddressMode address) noexcept;

    Builder& min_filter(VkFilter filter) noexcept;
    Builder& mag_filter(VkFilter filter) noexcept;

    Builder& mipmap_mode(VkSamplerMipmapMode mipmap_mode) noexcept;
    Builder& lod(std::pair<f32, f32> range) noexcept;
    Builder& max_anisotrophy(f32 value) noexcept;

private:
    VkFilter min_filter_ = VK_FILTER_NEAREST;
    VkFilter mag_filter_ = VK_FILTER_NEAREST;
    struct {
        VkSamplerAddressMode u = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        VkSamplerAddressMode v = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        VkSamplerAddressMode w = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    } address_mode_;
    VkSamplerMipmapMode mipmap_mode_ = {};
    std::pair<f32, f32> lod_range_   = {};
    f32 max_anisotrophy_             = {};
};

} // namespace texture_sampler

class TextureSampler {
public:
    using builder_type = texture_sampler::Builder;

    static builder_type start_build() noexcept;

    TextureSampler() noexcept = default;
    TextureSampler(VkDevice device, VkSampler sampler, VkSamplerCreateInfo create_info) noexcept;

    TextureSampler(const TextureSampler&) noexcept            = delete;
    TextureSampler& operator=(const TextureSampler&) noexcept = delete;

    TextureSampler(TextureSampler&&) noexcept;
    TextureSampler& operator=(TextureSampler&&) noexcept;
    ~TextureSampler() noexcept;

    operator VkSampler() const noexcept { return get(); }
    VkSampler get() const noexcept { return sampler_; }

private:
    VkDevice device_                 = nullptr;
    VkSampler sampler_               = nullptr;
    VkSamplerCreateInfo create_info_ = {};
};

} // namespace zoo::render::resources
