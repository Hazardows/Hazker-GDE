#pragma once

#include "renderer/vulkan/vk_types.inl"

void createVulkanImage(
    vulkanContext* context,
    VkImageType imageType,
    u32 width,
    u32 height,
    VkFormat format,
    VkImageTiling tiling,
    VkImageUsageFlags usage,
    VkMemoryPropertyFlags memory_flags,
    b32 create_view,
    VkImageAspectFlags view_aspect_flags,
    vulkanImage* output_image
);

void createVulkanImageView(
    vulkanContext* context,
    VkFormat format,
    vulkanImage* image,
    VkImageAspectFlags aspect_flags
);

void destroyVulkanImage(
    vulkanContext* context,
    vulkanImage* image
);