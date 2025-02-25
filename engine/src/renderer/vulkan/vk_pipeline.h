#pragma once

#include "renderer/vulkan/vk_types.inl"

b8 vk_create_graphics_pipeline(
    vulkanContext* context,
    vulkanRenderPass* renderPass,
    u32 attribute_count,
    VkVertexInputAttributeDescription* attributes,
    u32 descriptor_set_layout_count,
    VkDescriptorSetLayout* descriptor_set_layouts,
    u32 stage_count,
    VkPipelineShaderStageCreateInfo* stages,
    VkViewport viewport,
    VkRect2D scissor,
    b8 is_wireframe,
    vulkanPipeline* out_pipeline
);

void vk_destroy_graphics_pipeline(
    vulkanContext* context,
    vulkanPipeline* pipeline
);

void vk_bind_pipeline(
    vulkanCommandBuffer* command_buffer, 
    VkPipelineBindPoint bind_point, 
    vulkanPipeline* pipeline
);