#pragma once

#include "renderer/vulkan/vk_types.inl"

void createVulkanSwapchain(
    vulkanContext* context,
    u32 width,
    u32 height,
    vulkanSwapchain* outSwapchain
);

void recreateVulkanSwapchain(
    vulkanContext* context,
    u32 width,
    u32 height,
    vulkanSwapchain* swapchain
);

void destroyVulkanSwapchain(
    vulkanContext* context,
    vulkanSwapchain* swapchain
);

b8 vk_swapchain_acquire_next_image_index(
    vulkanContext* context,
    vulkanSwapchain* swapchain,
    u64 timeout_ns,
    VkSemaphore image_aviable_semaphore,
    VkFence fence,
    u32 *out_image_index
);

void vk_swapchain_present(
    vulkanContext* context,
    vulkanSwapchain* swapchain,
    VkQueue graphics_queue,
    VkQueue present_queue,
    VkSemaphore render_complete_semaphore,
    u32 present_image_index
);