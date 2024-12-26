#pragma once

#include "renderer/vulkan/vk_types.inl"

void createVulkanFence(
    vulkanContext* context, 
    b8 createSignaled,
    vulkanFence* outFence
);

void destroyVulkanFence(vulkanContext* context, vulkanFence* fence);

b8 vulkanFenceWait(vulkanContext* context, vulkanFence* fence, u64 timeout_ns);

void vulkanFenceReset(vulkanContext* context, vulkanFence* fence);