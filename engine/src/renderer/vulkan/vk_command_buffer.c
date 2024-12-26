#include "renderer/vulkan/vk_command_buffer.h"

#include "core/hmemory.h"

void vulkanCommandBufferAllocate(
    vulkanContext *context,
    VkCommandPool pool,
    b8 is_primary,
    vulkanCommandBuffer* out_command_buffer) {

    HzeroMemory(out_command_buffer, sizeof(out_command_buffer));

    VkCommandBufferAllocateInfo alloc_info = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
    alloc_info.commandPool = pool;
    alloc_info.level = is_primary ? VK_COMMAND_BUFFER_LEVEL_PRIMARY : VK_COMMAND_BUFFER_LEVEL_SECONDARY;
    alloc_info.commandBufferCount = 1;
    alloc_info.pNext = NULL;

    out_command_buffer->state = COMMAND_BUFFER_STATE_NOT_ALLOCATED;
    VK_CHECK(vkAllocateCommandBuffers(
        context->device.logical_device,
        &alloc_info,
        &out_command_buffer->handle
    ));
    out_command_buffer->state = COMMAND_BUFFER_STATE_READY;
}

void vulkanCommandBufferFree(
    vulkanContext* context,
    VkCommandPool pool,
    vulkanCommandBuffer* command_buffer) {

    vkFreeCommandBuffers(
        context->device.logical_device,
        pool,
        1,
        &command_buffer->handle
    );
    command_buffer->handle = NULL;
    command_buffer->state = COMMAND_BUFFER_STATE_NOT_ALLOCATED;
}

void vulkanCommandBufferBegin(
    vulkanCommandBuffer* command_buffer,
    b8 is_single_use,
    b8 is_renderpass_continue,
    b8 is_simultaneous_use) {

    VkCommandBufferBeginInfo begin_info = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    begin_info.flags = 0;
    if (is_single_use) {
        begin_info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    }
    if (is_renderpass_continue) {
        begin_info.flags |= VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    }
    if (is_simultaneous_use) {
        begin_info.flags |= VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    }

    VK_CHECK(vkBeginCommandBuffer(command_buffer->handle, &begin_info));
    command_buffer->state = COMMAND_BUFFER_STATE_RECORDING;
}

void vulkanCommandBufferEnd(vulkanCommandBuffer* command_buffer) {
    VK_CHECK(vkEndCommandBuffer(command_buffer->handle));
    command_buffer->state = COMMAND_BUFFER_STATE_RECORDING_ENDED;
}

void vulkanCommandBufferUpdateSubmited(vulkanCommandBuffer* command_buffer) {
    command_buffer->state = COMMAND_BUFFER_STATE_SUBMITTED;
}

void vulkanCommandBufferReset(vulkanCommandBuffer* command_buffer) {
    command_buffer->state = COMMAND_BUFFER_STATE_READY;
}

void vulkanCommandBufferAllocateAndBeginSingleUse(
    vulkanContext *context,
    VkCommandPool pool,
    vulkanCommandBuffer* out_command_buffer) {

    vulkanCommandBufferAllocate(context, pool, true, out_command_buffer);
    vulkanCommandBufferBegin(out_command_buffer, true, false, false);
}

void vulkanCommandBufferEndSingleUse(
    vulkanContext* context,
    VkCommandPool pool,
    vulkanCommandBuffer* command_buffer,
    VkQueue queue) {
    
    // End the command buffer
    vulkanCommandBufferEnd(command_buffer);

    // Submit the queue
    VkSubmitInfo submit_info = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &command_buffer->handle;
    VK_CHECK(vkQueueSubmit(queue, 1, &submit_info, NULL));

    // Wait for it to finish
    VK_CHECK(vkQueueWaitIdle(queue));

    // Free the command buffer
    vulkanCommandBufferFree(context, pool, command_buffer);
}