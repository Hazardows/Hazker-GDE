#pragma once

#include "renderer/vulkan/vk_types.inl"

b8 vulkanBuffer_create(
    vulkanContext* context,
    u64 size,
    VkBufferUsageFlagBits usage,
    u32 memory_property_flags,
    b8 bind_on_create,
    vulkanBuffer* out_buffer
);

void vulkanBuffer_destroy(vulkanContext* context, vulkanBuffer* buffer);

b8 vulkanBuffer_resize(
    vulkanContext* context,
    u64 new_size,
    vulkanBuffer* buffer,
    VkQueue queue,
    VkCommandPool pool 
);

void vulkanBuffer_bind(vulkanContext* context, vulkanBuffer* buffer, u64 offset);

void* vulkanBuffer_lock_memory(vulkanContext* context, vulkanBuffer* buffer, u64 offset, u64 size, u32 flags);
void vulkanBuffer_unlock_memory(vulkanContext* context, vulkanBuffer* buffer);

void vulkanBuffer_load_data(vulkanContext* context, vulkanBuffer* buffer, u64 offset, u64 size, u32 flags, void* data);

void vulkanBuffer_copy_to(
    vulkanContext* context,
    VkCommandPool pool,
    VkFence fence, 
    VkQueue queue,
    VkBuffer source,
    u64 source_offset,
    VkBuffer dest,
    u64 dest_offset,
    u64 size
);