// Device Information
#pragma once

#include "renderer/vulkan/vk_types.inl"

b8 createVulkanDevice(vulkanContext *context);
void destroyVulkanDevice(vulkanContext *context);

void vulkanDeviceQuerySwapChainSupport(
    VkPhysicalDevice physical_device,
    VkSurfaceKHR surface,
    vk_swapchain_support_info* out_support_info
);

b8 vulkanDeviceDetectDepthFormat(vulkanDevice* device);