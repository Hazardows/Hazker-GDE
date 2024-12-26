#pragma once

#include "renderer/vulkan/vk_types.inl"

void vulkanFramebufferCreate(
    vulkanContext* context,
    vulkanRenderPass* renderPass,
    u32 width,
    u32 height,
    u32 attachmentCount,
    VkImageView* attachments,
    vulkanFramebuffer* out_framebuffer
);

void vulkanFramebufferDestroy(vulkanContext* context, vulkanFramebuffer* framebuffer);