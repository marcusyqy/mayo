#include "Render.hpp"

#include "Defines.hpp"
#include "render/fwd.hpp"

// Our renderer contexts.
#include "render/DescriptorPool.hpp"
#include "render/DeviceContext.hpp"
#include "render/Engine.hpp"
#include "render/Pipeline.hpp"
#include "render/resources/Buffer.hpp"
#include "render/resources/Texture.hpp"
#include "render/scene/UploadContext.hpp"
#include "render/sync/Fence.hpp"

#include "adapters/tools/ShaderCompiler.hpp"

#include "imgui.h"
#include <stdio.h>

// TODO: Determine if we need this.
// Visual Studio warnings
#ifdef _MSC_VER
#pragma warning(disable : 4127) // condition expression is constant
#endif

namespace zoo::adapters::imgui {

namespace {

// @Brittle this needs to be aligned with the imgui layer.
struct PushConstantData {
    float scale[2];
    float translate[2];
};

struct Imgui_Frame_Data {
    render::scene::CommandBuffer command_buffer; // this will probably not be needed as well.
    render::ResourceBindings bindings;
    render::sync::Fence fence;
    render::Framebuffer render_target;

    // Don't initialize this stuff.
    render::resources::Buffer vertex = {};
    render::resources::Buffer index  = {};
};

struct Imgui_Deleter {
    const bool owned = true;
    void operator()(render::Swapchain* sc) {
        if (owned && sc) delete sc;
    }
};

struct Imgui_Viewport_Data {
    static constexpr auto MAX_FRAMES = 3;
    // the destructor will take care of the deallocation for the reference we will pass in something that doesn't
    // deallocate.
    std::unique_ptr<render::Swapchain, Imgui_Deleter> swapchain;
    Imgui_Frame_Data frame[MAX_FRAMES];
};

struct Imgui_Vulkan_Data {
    render::Engine& engine;
    render::DeviceContext& context;
    render::scene::UploadContext upload_context;

    render::resources::Texture font_tex;
    render::resources::TextureSampler font_sampler;

    render::RenderPass renderpass;
    render::Pipeline pipeline;
    render::DescriptorPool descriptor_pool;

    Imgui_Viewport_Data* main_window_data;

    // @TODO: may have to dynamically set this
    static constexpr auto BUFFER_MEMORY_ALIGNMENT = 256;
};

// For ease of convenience.
Imgui_Vulkan_Data& imgui_get_render_static_data() {
    Imgui_Vulkan_Data* data =
        ImGui::GetCurrentContext() ? (Imgui_Vulkan_Data*)ImGui::GetIO().BackendRendererUserData : nullptr;
    ZOO_ASSERT(data != nullptr);
    return *data;
}

template <typename T>
render::resources::Buffer imgui_create_buffer(size_t count, VkBufferUsageFlagBits usage) {
    auto& bd = imgui_get_render_static_data();

    // @NOTE: Don't think we need to align the memory. We can assume that VMA does it for us.
    // VkDeviceSize vertex_buffer_size_aligned =
    //     ((new_size - 1) / bd->BUFFER_MEMORY_ALIGNMENT + 1) * bd->BUFFER_MEMORY_ALIGNMENT;
    return render::resources::Buffer::start_build<T>("Imgui buffer")
        .usage(usage)
        .count(count)
        .allocation_type(VMA_MEMORY_USAGE_AUTO)
        .allocation_flag(VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT)
        .build(bd.context.allocator());
}

render::PushConstant imgui_get_push_constant_descriptor() {
    return { .stageFlags = VK_SHADER_STAGE_VERTEX_BIT, .offset = 0, .size = sizeof(PushConstantData) };
}

// @NOTE: uncomment to implement.
// void imgui_create_window(ImGuiViewport* viewport) {
//     // @TODO : Finish this function.
//     Imgui_Vulkan_Data* bd        = imgui_get_render_static_data();
//     Imgui_Viewport_Data* vd      = IM_NEW(Imgui_Viewport_Data)();
//     viewport->RendererUserData   = vd;
//     ImGui_ImplVulkanH_Window* wd = &vd->Window;
//     ImGui_ImplVulkan_InitInfo* v = &bd->VulkanInitInfo;
//
//     // Create surface
//     ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();
//     VkResult err                 = (VkResult)platform_io.Platform_CreateVkSurface(
//         viewport,
//         (ImU64)v->Instance,
//         (const void*)v->Allocator,
//         (ImU64*)&wd->Surface);
//     check_vk_result(err);
//
//     // Check for WSI support
//     VkBool32 res;
//     vkGetPhysicalDeviceSurfaceSupportKHR(v->PhysicalDevice, v->QueueFamily, wd->Surface, &res);
//     if (res != VK_TRUE) {
//         IM_ASSERT(0); // Error: no WSI support on physical device
//         return;
//     }
//
//     // Select Surface Format
//     const VkFormat requestSurfaceImageFormat[]     = { VK_FORMAT_B8G8R8A8_UNORM,
//                                                        VK_FORMAT_R8G8B8A8_UNORM,
//                                                        VK_FORMAT_B8G8R8_UNORM,
//                                                        VK_FORMAT_R8G8B8_UNORM };
//     const VkColorSpaceKHR requestSurfaceColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
//     wd->SurfaceFormat                              = ImGui_ImplVulkanH_SelectSurfaceFormat(
//         v->PhysicalDevice,
//         wd->Surface,
//         requestSurfaceImageFormat,
//         (size_t)IM_ARRAYSIZE(requestSurfaceImageFormat),
//         requestSurfaceColorSpace);
//
//     // Select Present Mode
//     // FIXME-VULKAN: Even thought mailbox seems to get us maximum framerate with a single window, it halves framerate
//     // with a second window etc. (w/ Nvidia and SDK 1.82.1)
//     VkPresentModeKHR present_modes[] = { VK_PRESENT_MODE_MAILBOX_KHR,
//                                          VK_PRESENT_MODE_IMMEDIATE_KHR,
//                                          VK_PRESENT_MODE_FIFO_KHR };
//     wd->PresentMode                  = ImGui_ImplVulkanH_SelectPresentMode(
//         v->PhysicalDevice,
//         wd->Surface,
//         &present_modes[0],
//         IM_ARRAYSIZE(present_modes));
//     // printf("[vulkan] Secondary window selected PresentMode = %d\n", wd->PresentMode);
//
//     // Create SwapChain, RenderPass, Framebuffer, etc.
//     wd->ClearEnable         = (viewport->Flags & ImGuiViewportFlags_NoRendererClear) ? false : true;
//     wd->UseDynamicRendering = v->UseDynamicRendering;
//     ImGui_ImplVulkanH_CreateOrResizeWindow(
//         v->Instance,
//         v->PhysicalDevice,
//         v->Device,
//         wd,
//         v->QueueFamily,
//         v->Allocator,
//         (int)viewport->Size.x,
//         (int)viewport->Size.y,
//         v->MinImageCount);
//     vd->WindowOwned = true;
// }
//
//
// void imgui_destroy_window(ImGuiViewport* viewport) {
//     // The main viewport (owned by the application) will always have RendererUserData == 0 since we didn't create the
//     // data for it.
//     ImGui_ImplVulkan_Data* bd = ImGui_ImplVulkan_GetBackendData();
//     if (ImGui_ImplVulkan_ViewportData* vd = (ImGui_ImplVulkan_ViewportData*)viewport->RendererUserData) {
//         ImGui_ImplVulkan_InitInfo* v = &bd->VulkanInitInfo;
//         if (vd->WindowOwned) ImGui_ImplVulkanH_DestroyWindow(v->Instance, v->Device, &vd->Window, v->Allocator);
//         ImGui_ImplVulkanH_DestroyWindowRenderBuffers(v->Device, &vd->RenderBuffers, v->Allocator);
//         IM_DELETE(vd);
//     }
//     viewport->RendererUserData = nullptr;
// }
//
// void imgui_window_resize(ImGuiViewport* viewport, ImVec2 size) {
//     ImGui_ImplVulkan_Data* bd         = ImGui_ImplVulkan_GetBackendData();
//     ImGui_ImplVulkan_ViewportData* vd = (ImGui_ImplVulkan_ViewportData*)viewport->RendererUserData;
//     if (vd == nullptr) // This is nullptr for the main viewport (which is left to the user/app to handle)
//         return;
//     ImGui_ImplVulkan_InitInfo* v = &bd->VulkanInitInfo;
//     vd->Window.ClearEnable       = (viewport->Flags & ImGuiViewportFlags_NoRendererClear) ? false : true;
//     ImGui_ImplVulkanH_CreateOrResizeWindow(
//         v->Instance,
//         v->PhysicalDevice,
//         v->Device,
//         &vd->Window,
//         v->QueueFamily,
//         v->Allocator,
//         (int)size.x,
//         (int)size.y,
//         v->MinImageCount);
// }
//
// void imgui_render_window(ImGuiViewport* viewport, void*) {
//     ImGui_ImplVulkan_Data* bd         = ImGui_ImplVulkan_GetBackendData();
//     ImGui_ImplVulkan_ViewportData* vd = (ImGui_ImplVulkan_ViewportData*)viewport->RendererUserData;
//     ImGui_ImplVulkanH_Window* wd      = &vd->Window;
//     ImGui_ImplVulkan_InitInfo* v      = &bd->VulkanInitInfo;
//     VkResult err;
//
//     ImGui_ImplVulkanH_Frame* fd            = &wd->Frames[wd->FrameIndex];
//     ImGui_ImplVulkanH_FrameSemaphores* fsd = &wd->FrameSemaphores[wd->SemaphoreIndex];
//     {
//         {
//             err = vkAcquireNextImageKHR(
//                 v->Device,
//                 wd->Swapchain,
//                 UINT64_MAX,
//                 fsd->ImageAcquiredSemaphore,
//                 VK_NULL_HANDLE,
//                 &wd->FrameIndex);
//             check_vk_result(err);
//             fd = &wd->Frames[wd->FrameIndex];
//         }
//         for (;;) {
//             err = vkWaitForFences(v->Device, 1, &fd->Fence, VK_TRUE, 100);
//             if (err == VK_SUCCESS) break;
//             if (err == VK_TIMEOUT) continue;
//             check_vk_result(err);
//         }
//         {
//             err = vkResetCommandPool(v->Device, fd->CommandPool, 0);
//             check_vk_result(err);
//             VkCommandBufferBeginInfo info = {};
//             info.sType                    = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
//             info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
//             err = vkBeginCommandBuffer(fd->CommandBuffer, &info);
//             check_vk_result(err);
//         }
//         {
//             ImVec4 clear_color = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
//             memcpy(&wd->ClearValue.color.float32[0], &clear_color, 4 * sizeof(float));
//         }
// #ifdef IMGUI_IMPL_VULKAN_HAS_DYNAMIC_RENDERING
//         if (v->UseDynamicRendering) {
//             // Transition swapchain image to a layout suitable for drawing.
//             VkImageMemoryBarrier barrier        = {};
//             barrier.sType                       = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
//             barrier.dstAccessMask               = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
//             barrier.oldLayout                   = VK_IMAGE_LAYOUT_UNDEFINED;
//             barrier.newLayout                   = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
//             barrier.image                       = fd->Backbuffer;
//             barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
//             barrier.subresourceRange.levelCount = 1;
//             barrier.subresourceRange.layerCount = 1;
//             vkCmdPipelineBarrier(
//                 fd->CommandBuffer,
//                 VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
//                 VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
//                 0,
//                 0,
//                 nullptr,
//                 0,
//                 nullptr,
//                 1,
//                 &barrier);
//
//             VkRenderingAttachmentInfo attachmentInfo = {};
//             attachmentInfo.sType                     = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
//             attachmentInfo.imageView                 = fd->BackbufferView;
//             attachmentInfo.imageLayout               = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
//             attachmentInfo.resolveMode               = VK_RESOLVE_MODE_NONE;
//             attachmentInfo.loadOp                    = VK_ATTACHMENT_LOAD_OP_CLEAR;
//             attachmentInfo.storeOp                   = VK_ATTACHMENT_STORE_OP_STORE;
//             attachmentInfo.clearValue                = wd->ClearValue;
//
//             VkRenderingInfo renderingInfo          = {};
//             renderingInfo.sType                    = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR;
//             renderingInfo.renderArea.extent.width  = wd->Width;
//             renderingInfo.renderArea.extent.height = wd->Height;
//             renderingInfo.layerCount               = 1;
//             renderingInfo.viewMask                 = 0;
//             renderingInfo.colorAttachmentCount     = 1;
//             renderingInfo.pColorAttachments        = &attachmentInfo;
//
//             ImGuiImplVulkanFuncs_vkCmdBeginRenderingKHR(fd->CommandBuffer, &renderingInfo);
//         } else
// #endif
//         {
//             VkRenderPassBeginInfo info    = {};
//             info.sType                    = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
//             info.renderPass               = wd->RenderPass;
//             info.framebuffer              = fd->Framebuffer;
//             info.renderArea.extent.width  = wd->Width;
//             info.renderArea.extent.height = wd->Height;
//             info.clearValueCount          = (viewport->Flags & ImGuiViewportFlags_NoRendererClear) ? 0 : 1;
//             info.pClearValues = (viewport->Flags & ImGuiViewportFlags_NoRendererClear) ? nullptr : &wd->ClearValue;
//             vkCmdBeginRenderPass(fd->CommandBuffer, &info, VK_SUBPASS_CONTENTS_INLINE);
//         }
//     }
//
//     imgui_render_impl(viewport->DrawData, fd->CommandBuffer, wd->Pipeline);
//
//     {
// #ifdef IMGUI_IMPL_VULKAN_HAS_DYNAMIC_RENDERING
//         if (v->UseDynamicRendering) {
//             ImGuiImplVulkanFuncs_vkCmdEndRenderingKHR(fd->CommandBuffer);
//
//             // Transition image to a layout suitable for presentation
//             VkImageMemoryBarrier barrier        = {};
//             barrier.sType                       = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
//             barrier.srcAccessMask               = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
//             barrier.oldLayout                   = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
//             barrier.newLayout                   = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
//             barrier.image                       = fd->Backbuffer;
//             barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
//             barrier.subresourceRange.levelCount = 1;
//             barrier.subresourceRange.layerCount = 1;
//             vkCmdPipelineBarrier(
//                 fd->CommandBuffer,
//                 VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
//                 VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
//                 0,
//                 0,
//                 nullptr,
//                 0,
//                 nullptr,
//                 1,
//                 &barrier);
//         } else
// #endif
//         {
//             vkCmdEndRenderPass(fd->CommandBuffer);
//         }
//         {
//             VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
//             VkSubmitInfo info               = {};
//             info.sType                      = VK_STRUCTURE_TYPE_SUBMIT_INFO;
//             info.waitSemaphoreCount         = 1;
//             info.pWaitSemaphores            = &fsd->ImageAcquiredSemaphore;
//             info.pWaitDstStageMask          = &wait_stage;
//             info.commandBufferCount         = 1;
//             info.pCommandBuffers            = &fd->CommandBuffer;
//             info.signalSemaphoreCount       = 1;
//             info.pSignalSemaphores          = &fsd->RenderCompleteSemaphore;
//
//             err = vkEndCommandBuffer(fd->CommandBuffer);
//             check_vk_result(err);
//             err = vkResetFences(v->Device, 1, &fd->Fence);
//             check_vk_result(err);
//             err = vkQueueSubmit(v->Queue, 1, &info, fd->Fence);
//             check_vk_result(err);
//         }
//     }
// }
//
// void imgui_swapbuffers(ImGuiViewport* viewport, void*) {
//     auto* vd = (Imgui_Viewport_Data*)viewport->RendererUserData;
//     // Probably just need to this.
//     vd->swapchain.present();
// }

void imgui_attach_viewport_callbacks() {
    ZOO_LOG_ERROR("imgui_attach_viewport_callbacks-not implemented");
    // @TODO finish all the created functions
    ImGuiPlatformIO& platform_io       = ImGui::GetPlatformIO();
    platform_io.Renderer_CreateWindow  = nullptr; // imgui_create_window;
    platform_io.Renderer_DestroyWindow = nullptr; // imgui_destroy_window;
    platform_io.Renderer_SetWindowSize = nullptr; // imgui_window_resize;
    platform_io.Renderer_RenderWindow  = nullptr; // imgui_render_window;
    platform_io.Renderer_SwapBuffers   = nullptr; // imgui_swapbuffers;
}

void imgui_destroy_viewports() {
    // @TODO : work on this.
    ZOO_LOG_ERROR("`imgui_destroy_viewports` := this is not really implemented well as well.");
    ImGui::DestroyPlatformWindows();
}

Imgui_Frame_Data imgui_create_frame_data(
    Imgui_Vulkan_Data& vkdata,
    render::Swapchain& swapchain,
    s32 index,
    u32 width,
    u32 height,
    Imgui_Frame_Data* old_data = nullptr) {
    auto& context       = vkdata.context;
    auto command_buffer = old_data ? std::move(old_data->command_buffer)
                                   : render::scene::CommandBuffer{ context, render::Operation::graphics };
    auto bindings       = old_data ? std::move(old_data->bindings) : [&vkdata]() {
        auto b = vkdata.descriptor_pool.allocate(vkdata.pipeline);
        b.start_batch()
            .bind(0, vkdata.font_tex, vkdata.font_sampler, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
            .end_batch();
        return b;
    }();

    auto fence = old_data ? std::move(old_data->fence) : render::sync::Fence{ context, true };
    const render::resources::TextureView* tv[] = { swapchain.get_image(index) };
    auto render_target                         = render::Framebuffer{ context, vkdata.renderpass, tv, width, height };

    const auto default_init_size = 100;
    auto vertex_buffer           = old_data
                  ? std::move(old_data->vertex)
                  : imgui_create_buffer<ImDrawVert>(default_init_size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    auto index_buffer            = old_data ? std::move(old_data->index)
                                            : imgui_create_buffer<ImDrawIdx>(default_init_size, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
    return { .command_buffer = std::move(command_buffer),
             .bindings       = std::move(bindings),
             .fence          = std::move(fence),
             .render_target  = std::move(render_target),
             .vertex         = std::move(vertex_buffer),
             .index          = std::move(index_buffer) };
}

render::RenderPass imgui_create_renderpass(render::DeviceContext& context, VkFormat image_format) {
    render::AttachmentDescription attachments[] = { render::ColorAttachmentDescription(image_format) };
    return { context, attachments };
}

render::Pipeline imgui_create_pipeline(render::DeviceContext& context, const render::RenderPass& renderpass) {
    std::array buffer_description{
        render::VertexBufferDescription{ 0, render::ShaderType::vec2, offsetof(ImDrawVert, pos) },
        render::VertexBufferDescription{ 1, render::ShaderType::vec2, offsetof(ImDrawVert, uv) },
        render::VertexBufferDescription{ 2, render::ShaderType::vec4_unorm, offsetof(ImDrawVert, col) }
    };

    std::array vertex_description{
        render::VertexInputDescription{ sizeof(ImDrawVert), buffer_description, VK_VERTEX_INPUT_RATE_VERTEX }
    };

    tools::ShaderCompiler shader_compiler;
    render::Shader vertex_shader{ context, __glsl_shader_vert_spv, "main" };
    render::Shader fragment_shader{ context, __glsl_shader_frag_spv, "main" };

    render::BindingDescriptor binding_descriptors[] = {
        { .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, .count = 1, .stage = VK_SHADER_STAGE_FRAGMENT_BIT }
    };

    render::PushConstant push_constant_info = imgui_get_push_constant_descriptor();

    render::PipelineCreateInfo pipeline_create_info{ false };

    return render::Pipeline{ context,
                             render::ShaderStagesSpecification{ vertex_shader, fragment_shader, vertex_description },
                             renderpass,
                             binding_descriptors,
                             &push_constant_info,
                             pipeline_create_info };
}

void imgui_init_pipeline_and_descriptors(Imgui_Vulkan_Data& data, VkFormat format) {
    auto& device_ctx     = data.context;
    data.renderpass      = imgui_create_renderpass(device_ctx, format);
    data.pipeline        = imgui_create_pipeline(device_ctx, data.renderpass);
    data.descriptor_pool = render::DescriptorPool{ device_ctx, 1000 };
}

render::resources::Texture
    init_font_textures(render::scene::UploadContext& upload_ctx, render::DeviceContext& device_ctx) noexcept {
    ImGuiIO& io = ImGui::GetIO();

    unsigned char* pixels;
    int width, height;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

    render::resources::Buffer scratch_buffer =
        render::resources::Buffer::start_build("ScratchBuffer for ImGui:Font", width * height * 4)
            .usage(VK_BUFFER_USAGE_TRANSFER_SRC_BIT)
            .allocation_type(VMA_MEMORY_USAGE_AUTO_PREFER_HOST)
            .allocation_flag(VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT)
            .build(device_ctx.allocator());

    auto map = scratch_buffer.map();
    memcpy(map, pixels, scratch_buffer.allocated_size());
    scratch_buffer.unmap();

    auto tex = render::resources::Texture::start_build("ImGui:Font")
                   .mip(1)
                   .array(1)
                   .extent({ .width = static_cast<u32>(width), .height = static_cast<u32>(height), .depth = 1 })
                   .samples(VK_SAMPLE_COUNT_1_BIT)
                   .type(VK_IMAGE_TYPE_2D)
                   .format(VK_FORMAT_R8G8B8A8_UNORM)
                   .usage(VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT)
                   .tiling(VK_IMAGE_TILING_OPTIMAL)
                   .allocation_type(VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE)
                   .allocation_required_flags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
                   .build(device_ctx.allocator());

    upload_ctx.copy(scratch_buffer, tex);
    upload_ctx.transition_to_shader_read(tex);
    upload_ctx.cache(std::move(scratch_buffer));

    upload_ctx.submit();
    // Store our identifier
    // io.Fonts->SetTexID((ImTextureID)bd->FontDescriptorSet);

    return tex;
}

void imgui_init_static_render_objects(Imgui_Vulkan_Data& vk_data, VkFormat image_format) {
    // create uploader.
    vk_data.upload_context = { vk_data.context };

    // load fonts
    vk_data.font_tex     = init_font_textures(vk_data.upload_context, vk_data.context);
    vk_data.font_sampler = render::resources::TextureSampler::start_build()
                               .address_mode(VK_SAMPLER_ADDRESS_MODE_REPEAT)
                               .min_filter(VK_FILTER_LINEAR)
                               .mag_filter(VK_FILTER_LINEAR)
                               .mipmap_mode(VK_SAMPLER_MIPMAP_MODE_LINEAR)
                               .lod(std::make_pair(-1000.f, 1000.f))
                               .max_anisotrophy(1.f)
                               .build(vk_data.context);

    // @TODO: We need to change renderdata
    imgui_init_pipeline_and_descriptors(vk_data, image_format);
}

} // namespace

void imgui_init(render::Engine& engine, render::DeviceContext& context, render::Swapchain& main_swapchain) {

    ImGuiIO& io = ImGui::GetIO();
    IM_ASSERT(io.BackendRendererUserData == nullptr && "Already initialized a renderer backend!");

    // @Cleanup: this has to change when swapchain doesn't hold the renderpass anymore
    Imgui_Vulkan_Data* vk_data = new Imgui_Vulkan_Data{ .engine = engine, .context = context };

    // set backend values flags
    io.BackendRendererUserData = (void*)vk_data;
    io.BackendRendererName     = "Zoo Vulkan Imgui";
    io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset; // We can honor the ImDrawCmd::VtxOffset field, allowing
                                                               // for large meshes.
    io.BackendFlags |=
        ImGuiBackendFlags_RendererHasViewports; // We can create multi-viewports on the Renderer side (optional)

    imgui_init_static_render_objects(*vk_data, main_swapchain.format());

    // Creation of initial main window.
    ImGuiViewport* main_viewport    = ImGui::GetMainViewport();
    main_viewport->RendererUserData = vk_data->main_window_data =
        new Imgui_Viewport_Data{ .swapchain = { &main_swapchain, Imgui_Deleter{ false } } };

    // this will not be null.
    auto& main_window_data = *vk_data->main_window_data;

    auto& swapchain = *main_window_data.swapchain;

    auto populate_or_repopulate_frame_data = [](bool use_old_data) {
        return [use_old_data](render::Swapchain& swapchain, s32 x, s32 y) {
            auto& vd       = imgui_get_render_static_data();
            s32 num_images = swapchain.num_images();
            ZOO_ASSERT(num_images <= Imgui_Viewport_Data::MAX_FRAMES);
            ZOO_ASSERT(vd.main_window_data);
            for (s32 i = 0; i < num_images; ++i)
                vd.main_window_data->frame[i] = imgui_create_frame_data(
                    vd,
                    swapchain,
                    i,
                    x,
                    y,
                    use_old_data ? &vd.main_window_data->frame[i] : nullptr);
        };
    };

    auto [x, y] = swapchain.extent();
    populate_or_repopulate_frame_data(false)(swapchain, x, y);
    swapchain.on_resize(populate_or_repopulate_frame_data(true));

    imgui_attach_viewport_callbacks();

    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) imgui_attach_viewport_callbacks();

    vk_data->upload_context.wait();
}

void imgui_exit() {
    Imgui_Vulkan_Data& bd = imgui_get_render_static_data();
    bd.context.wait(); // make sure device is idle.
    ImGuiIO& io = ImGui::GetIO();

    // Manually delete main viewport render data in-case we haven't initialized for viewports
    ImGuiViewport* main_viewport = ImGui::GetMainViewport();
    if (Imgui_Viewport_Data* vd = (Imgui_Viewport_Data*)main_viewport->RendererUserData) {
        delete vd;
        main_viewport->RendererUserData = nullptr;
    }

    imgui_destroy_viewports();

    // Clean up windows
    // ImGui_ImplVulkan_ShutdownPlatformInterface();

    io.BackendRendererName     = nullptr;
    io.BackendRendererUserData = nullptr;
    io.BackendFlags &= ~(ImGuiBackendFlags_RendererHasVtxOffset | ImGuiBackendFlags_RendererHasViewports);

    delete std::addressof(bd);
}

void imgui_setup_render_state(const ImDrawData& draw_data, Imgui_Frame_Data& fd, int fb_width, int fb_height) {
    auto& bd              = imgui_get_render_static_data();
    auto& command_context = fd.command_buffer;
    {
        PushConstantData pcd{};
        auto& scale     = pcd.scale;
        scale[0]        = 2.0f / draw_data.DisplaySize.x;
        scale[1]        = 2.0f / draw_data.DisplaySize.y;
        auto& translate = pcd.translate;
        translate[0]    = -1.0f - draw_data.DisplayPos.x * scale[0];
        translate[1]    = -1.0f - draw_data.DisplayPos.y * scale[1];
        command_context.bind_pipeline(bd.pipeline)
            .push_constants(imgui_get_push_constant_descriptor(), &pcd)
            .bindings(fd.bindings);
    }

    command_context.bind_vertex_buffers(&fd.vertex);
    command_context.bind_index_buffer(fd.index);

    // Setup viewport:
    {
        VkViewport viewport;
        viewport.x        = 0;
        viewport.y        = 0;
        viewport.width    = (float)fb_width;
        viewport.height   = (float)fb_height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        command_context.set_viewport(viewport);
    }
}
bool imgui_should_render(const ImDrawData& draw_data) {
    int fb_width  = (int)(draw_data.DisplaySize.x * draw_data.FramebufferScale.x);
    int fb_height = (int)(draw_data.DisplaySize.y * draw_data.FramebufferScale.y);
    return !(fb_width <= 0 || fb_height <= 0);
}

void imgui_render(const ImDrawData& draw_data, Imgui_Frame_Data& fd) {
    // Avoid rendering when minimized, scale coordinates for retina displays (screen coordinates != framebuffer
    // coordinates)
    if (!imgui_should_render(draw_data)) return;

    int fb_width          = (int)(draw_data.DisplaySize.x * draw_data.FramebufferScale.x);
    int fb_height         = (int)(draw_data.DisplaySize.y * draw_data.FramebufferScale.y);
    auto& command_context = fd.command_buffer;

    // @NOTE: we should begin renderpass here instead maybe.
    // If there is data to be drawn.
    if (draw_data.TotalVtxCount > 0) {
        // assure size is enough.
        if (!fd.vertex || fd.vertex.count() < (size_t)draw_data.TotalVtxCount)
            fd.vertex = imgui_create_buffer<ImDrawVert>(draw_data.TotalVtxCount, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);

        if (!fd.index || fd.index.count() < (size_t)draw_data.TotalIdxCount)
            fd.index = imgui_create_buffer<ImDrawIdx>(draw_data.TotalIdxCount, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

        // Upload vertex/index data into a single contiguous GPU buffer
        ImDrawVert* vtx_dst = fd.vertex.map<ImDrawVert>();
        ImDrawIdx* idx_dst  = fd.index.map<ImDrawIdx>();

        for (int n = 0; n < draw_data.CmdListsCount; n++) {
            const ImDrawList* cmd_list = draw_data.CmdLists[n];
            memcpy(vtx_dst, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
            memcpy(idx_dst, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
            vtx_dst += cmd_list->VtxBuffer.Size;
            idx_dst += cmd_list->IdxBuffer.Size;
        }

        // unmap both buffers after.
        fd.vertex.unmap();
        fd.index.unmap();
    }

    // Setup desired Vulkan state
    imgui_setup_render_state(draw_data, fd, fb_width, fb_height);

    // Will project scissor/clipping rectangles into framebuffer space
    ImVec2 clip_off   = draw_data.DisplayPos;       // (0,0) unless using multi-viewports
    ImVec2 clip_scale = draw_data.FramebufferScale; // (1,1) unless using retina display which are often (2,2)

    // Render command lists
    // (Because we merged all buffers into a single one, we maintain our own offset into them)
    int global_vtx_offset = 0;
    int global_idx_offset = 0;
    for (int n = 0; n < draw_data.CmdListsCount; n++) {
        const ImDrawList* cmd_list = draw_data.CmdLists[n];
        for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++) {
            const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
            // @TODO: I don't know what this is : we need to find out what this is doing.
            if (pcmd->UserCallback != nullptr) {
                // User callback, registered via ImDrawList::AddCallback()
                // (ImDrawCallback_ResetRenderState is a special callback value used by the user to request the renderer
                // to reset render state.)
                if (pcmd->UserCallback == ImDrawCallback_ResetRenderState)
                    imgui_setup_render_state(draw_data, fd, fb_width, fb_height);
                else
                    pcmd->UserCallback(cmd_list, pcmd);
            } else {
                // Project scissor/clipping rectangles into framebuffer space
                ImVec2 clip_min(
                    (pcmd->ClipRect.x - clip_off.x) * clip_scale.x,
                    (pcmd->ClipRect.y - clip_off.y) * clip_scale.y);
                ImVec2 clip_max(
                    (pcmd->ClipRect.z - clip_off.x) * clip_scale.x,
                    (pcmd->ClipRect.w - clip_off.y) * clip_scale.y);

                // Clamp to viewport as vkCmdSetScissor() won't accept values that are off bounds
                if (clip_min.x < 0.0f) {
                    clip_min.x = 0.0f;
                }
                if (clip_min.y < 0.0f) {
                    clip_min.y = 0.0f;
                }
                if (clip_max.x > fb_width) {
                    clip_max.x = (f32)fb_width;
                }
                if (clip_max.y > fb_height) {
                    clip_max.y = (f32)fb_height;
                }
                if (clip_max.x <= clip_min.x || clip_max.y <= clip_min.y) continue;

                // Apply scissor/clipping rectangle
                VkRect2D scissor;
                scissor.offset.x      = (s32)(clip_min.x);
                scissor.offset.y      = (s32)(clip_min.y);
                scissor.extent.width  = (u32)(clip_max.x - clip_min.x);
                scissor.extent.height = (u32)(clip_max.y - clip_min.y);
                command_context.set_scissor(scissor);

                // Bind DescriptorSet with font or user texture
                // @TODO: Looks like this is not needed.
                // VkDescriptorSet desc_set[1] = { (VkDescriptorSet)pcmd->TextureId };
                // if (sizeof(ImTextureID) < sizeof(ImU64)) {
                //     // We don't support texture switches if ImTextureID hasn't been redefined to be 64-bit. Do a
                //     flaky
                //     // check that other textures haven't been used.
                //     IM_ASSERT(pcmd->TextureId == (ImTextureID)bd->FontDescriptorSet);
                //     desc_set[0] = bd->FontDescriptorSet;
                // }

                // vkCmdBindDescriptorSets(
                //     command_buffer,
                //     VK_PIPELINE_BIND_POINT_GRAPHICS,
                //     bd->PipelineLayout,
                //     0,
                //     1,
                //     desc_set,
                //     0,
                //     nullptr);

                // Draw
                command_context.draw_indexed(
                    1,
                    pcmd->ElemCount,
                    pcmd->IdxOffset + global_idx_offset,
                    pcmd->VtxOffset + global_vtx_offset,
                    0);
            }
        }
        global_idx_offset += cmd_list->IdxBuffer.Size;
        global_vtx_offset += cmd_list->VtxBuffer.Size;
    }

    // Note: at this point both vkCmdSetViewport() and vkCmdSetScissor() have been called.
    // Our last values will leak into user/application rendering IF:
    // - Your app uses a pipeline with VK_DYNAMIC_STATE_VIEWPORT or VK_DYNAMIC_STATE_SCISSOR dynamic state
    // - And you forgot to call vkCmdSetViewport() and vkCmdSetScissor() yourself to explicitly set that state.
    // If you use VK_DYNAMIC_STATE_VIEWPORT or VK_DYNAMIC_STATE_SCISSOR you are responsible for setting the values
    // before rendering. In theory we should aim to backup/restore those values but I am not sure this is possible. We
    // perform a call to vkCmdSetScissor() to set back a full viewport which is likely to fix things for 99% users but
    // technically this is not perfect. (See github #4644)
    VkRect2D scissor = { { 0, 0 }, { (uint32_t)fb_width, (uint32_t)fb_height } };
    command_context.set_scissor(scissor);
}

void imgui_frame_render() {
    auto& vd = imgui_get_render_static_data();
    ZOO_ASSERT(vd.main_window_data);
    // wait for all uploads to be done.
    auto& viewport_data    = *vd.main_window_data;
    auto& swapchain        = *viewport_data.swapchain;
    const auto current_idx = swapchain.current_image();

    Imgui_Frame_Data& fd = viewport_data.frame[current_idx];
    fd.fence.wait();
    fd.fence.reset();

    auto& command_context      = fd.command_buffer;
    VkClearValue clear_color[] = { { { { 0.1f, 0.1f, 0.1f, 1.0f } } } };
    command_context.begin_renderpass(fd.render_target, clear_color);

    ImDrawData* main_draw_data = ImGui::GetDrawData();
    ZOO_ASSERT(main_draw_data != nullptr, "Something went wrong with ImGui drawdata.");
    imgui_render(*main_draw_data, fd);

    command_context.end_renderpass();
    command_context.submit(swapchain.current_present_context(), fd.fence);
}

void imgui_present() {
    auto& vd = imgui_get_render_static_data();
    ZOO_ASSERT(vd.main_window_data && vd.main_window_data->swapchain);
    ImDrawData* main_draw_data = ImGui::GetDrawData();
    ZOO_ASSERT(main_draw_data != nullptr, "Something went wrong with ImGui drawdata.");
    if (imgui_should_render(*main_draw_data)) vd.main_window_data->swapchain->present();
}

} // namespace zoo::adapters::imgui
