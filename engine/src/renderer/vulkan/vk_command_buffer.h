#pragma once

#include "renderer/vulkan/vk_types.inl"

void vulkanCommandBufferAllocate(
    vulkanContext *context,
    VkCommandPool pool,
    b8 is_primary,
    vulkanCommandBuffer* out_command_buffer
);

void vulkanCommandBufferFree(
    vulkanContext* context,
    VkCommandPool pool,
    vulkanCommandBuffer* command_buffer
);

void vulkanCommandBufferBegin(
    vulkanCommandBuffer* command_buffer,
    b8 is_single_use,
    b8 is_renderpass_continue,
    b8 is_simultaneous_use
);

void vulkanCommandBufferEnd(vulkanCommandBuffer* command_buffer);

void vulkanCommandBufferUpdateSubmited(vulkanCommandBuffer* command_buffer);

void vulkanCommandBufferReset(vulkanCommandBuffer* command_buffer);

/**
 *  Allocates and begins recording to out_command_buffer.
 */
void vulkanCommandBufferAllocateAndBeginSingleUse(
    vulkanContext *context,
    VkCommandPool pool,
    vulkanCommandBuffer* out_command_buffer
);

/**
 *  Ends recording, submits to and waits for queue operation and frees the provided command buffer.
 */
void vulkanCommandBufferEndSingleUse(
    vulkanContext* context,
    VkCommandPool pool,
    vulkanCommandBuffer* command_buffer,
    VkQueue queue
);