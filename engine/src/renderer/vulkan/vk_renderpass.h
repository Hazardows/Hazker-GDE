#pragma once

#include "renderer/vulkan/vk_types.inl"

void createVulkanRenderpass(
    vulkanContext *context,
    vulkanRenderPass *renderpass,
    f32 x, f32 y, f32 w, f32 h,     // Render area
    f32 r, f32 g, f32 b, f32 a,     // Color
    f32 depth, 
    u32 stencil
);

void destroyVulkanRenderpass(
    vulkanContext *context, 
    vulkanRenderPass *renderpass
);

void vulkanRenderPassBegin(
    vulkanCommandBuffer *command_buffer,
    vulkanRenderPass *renderpass,
    VkFramebuffer framebuffer
);

void vulkanRenderPassEnd(
    vulkanCommandBuffer *command_buffer,
    vulkanRenderPass *renderpass
);