#include "renderer/vulkan/vk_framebuffer.h"

#include "core/hmemory.h"

void vulkanFramebufferCreate(
    vulkanContext* context,
    vulkanRenderPass* renderPass,
    u32 width,
    u32 height,
    u32 attachmentCount,
    VkImageView* attachments,
    vulkanFramebuffer* out_framebuffer) {

    // Take a copy of the attachments, renderpass and attachment count
    out_framebuffer->attachments = Hallocate(sizeof(VkImageView) * attachmentCount, MEMORY_TAG_RENDERER);
    for (u32 i = 0; i < attachmentCount; i++) {
        out_framebuffer->attachments[i] = attachments[i];  
    }
    out_framebuffer->renderpass = renderPass;
    out_framebuffer->attachmentCount = attachmentCount;

    // Creation info
    VkFramebufferCreateInfo framebuffer_create_info = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
    framebuffer_create_info.renderPass = renderPass->handle;
    framebuffer_create_info.attachmentCount = attachmentCount;
    framebuffer_create_info.pAttachments = out_framebuffer->attachments;
    framebuffer_create_info.width = width;
    framebuffer_create_info.height = height;
    framebuffer_create_info.layers = 1;

    VK_CHECK(vkCreateFramebuffer(
        context->device.logical_device,
        &framebuffer_create_info,
        context->allocator,
        &out_framebuffer->handle
    ));
}

void vulkanFramebufferDestroy(vulkanContext* context, vulkanFramebuffer* framebuffer) {
    vkDestroyFramebuffer(context->device.logical_device, framebuffer->handle, context->allocator);
    if (framebuffer->attachments) {
        Hfree(framebuffer->attachments, sizeof(VkImageView) * framebuffer->attachmentCount, MEMORY_TAG_RENDERER);
        framebuffer->attachments = NULL;
    }
    framebuffer->handle = NULL;
    framebuffer->attachmentCount = 0;
    framebuffer->renderpass = NULL;
}