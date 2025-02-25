#include "vk_object_shader.h"

#include "core/logger.h"
#include "math/math_types.inl"
#include "memory/hmemory.h"

#include "renderer/vulkan/utils/vk_shader_utils.h"
#include "renderer/vulkan/vk_pipeline.h"
#include "renderer/vulkan/vk_buffer.h"

#define BUILTIN_SHADER_NAME_OBJECT "Builtin.ObjectShader"

b8 vk_create_object_shader(vulkanContext* context, vkObjectShader* out_shader) {
    // Shader module init per stage.
    char stage_type_strs[OBJECT_SHADER_STAGE_COUNT][5] = {"vert", "frag"};
    VkShaderStageFlagBits stage_types[OBJECT_SHADER_STAGE_COUNT] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT};

    for (u32 i = 0; i < OBJECT_SHADER_STAGE_COUNT; ++i) {
        if (!create_shader_module(context, BUILTIN_SHADER_NAME_OBJECT, stage_type_strs[i], stage_types[i], i, out_shader->stages)) {
            HERROR("Unable to create %s shader module for '%s'.", stage_type_strs[i], BUILTIN_SHADER_NAME_OBJECT);
            return false;
        }
    }

    // Global Descriptors
    VkDescriptorSetLayoutBinding global_ubo_layout_binding;
    global_ubo_layout_binding.binding = 0;
    global_ubo_layout_binding.descriptorCount = 1;
    global_ubo_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    global_ubo_layout_binding.pImmutableSamplers = NULL;
    global_ubo_layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutCreateInfo global_layout_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
    global_layout_info.bindingCount = 1;
    global_layout_info.pBindings = &global_ubo_layout_binding;
    
    VK_CHECK(vkCreateDescriptorSetLayout(
        context->device.logical_device,
        &global_layout_info, 
        context->allocator,
        &out_shader->globalDescriptorSetLayout
    ));

    // Global descriptor pool: Used for global items such as view/projection matrix.
    VkDescriptorPoolSize global_pool_size;
    global_pool_size.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    global_pool_size.descriptorCount = context->swapchain.imageCount;

    VkDescriptorPoolCreateInfo global_pool_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
    global_pool_info.poolSizeCount = 1;
    global_pool_info.pPoolSizes = &global_pool_size;
    global_pool_info.maxSets = context->swapchain.imageCount;

    VK_CHECK(vkCreateDescriptorPool(
        context->device.logical_device,
        &global_pool_info,
        context->allocator,
        &out_shader->globalDescriptorPool
    ));

    // Pipeline creation
    VkViewport viewport;
    viewport.x = 0.0f;
    viewport.y = (f32)context->framebuffer_height;
    viewport.width = (f32)context->framebuffer_width;
    viewport.height = -(f32)context->framebuffer_height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    // Scissor
    VkRect2D scissor;
    scissor.offset.x = scissor.offset.y = 0;
    scissor.extent.width = context->framebuffer_width;
    scissor.extent.height = context->framebuffer_height;

    // Attributes
    u32 offset = 0;
    #define attribute_count 1
    VkVertexInputAttributeDescription attribute_descriptions[attribute_count];
    // Position
    VkFormat formats[attribute_count] = {
        VK_FORMAT_R32G32B32_SFLOAT
    };
    u64 sizes[attribute_count] = {
        sizeof(vec3)
    };
    for (u32 i = 0; i < attribute_count; ++i) {
        attribute_descriptions[i].binding = 0;   // binding index - should match binding desc
        attribute_descriptions[i].location = i;  // attrib location
        attribute_descriptions[i].format = formats[i];
        attribute_descriptions[i].offset = offset;
        offset += sizes[i];
    }

    // Desciptor set layouts.
    #define descriptor_set_layout_count 1
    VkDescriptorSetLayout layouts[descriptor_set_layout_count] = {
        out_shader->globalDescriptorSetLayout
    };

    // Stages
    // NOTE: Should match the number of shader->stages.
    VkPipelineShaderStageCreateInfo stage_create_infos[OBJECT_SHADER_STAGE_COUNT];
    HzeroMemory(stage_create_infos, sizeof(stage_create_infos));
    for (u32 i = 0; i < OBJECT_SHADER_STAGE_COUNT; ++i) {
        stage_create_infos[i] = out_shader->stages[i].shaderStageCreateInfo;
        stage_create_infos[i].sType = out_shader->stages[i].shaderStageCreateInfo.sType;
    }

    if (!vk_create_graphics_pipeline(
            context,
            &context->main_renderpass,
            attribute_count,
            attribute_descriptions,
            descriptor_set_layout_count,
            layouts,
            OBJECT_SHADER_STAGE_COUNT,
            stage_create_infos,
            viewport,
            scissor,
            false,
            &out_shader->pipeline)) {
        HERROR("Failed to load graphics pipeline for object shader.");
        return false;
    }

    // Create uniform buffer
    if (!vulkanBuffer_create(
            context,
            sizeof(globalUniformObject) * 3,
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            true,
            &out_shader->globalUniformBuffer)) {
        HERROR("Vulkan buffer creation failed for object shader.");
        return false;
    }

    // Allocate global descriptor sets.
    VkDescriptorSetLayout global_layouts[3] = {
        out_shader->globalDescriptorSetLayout,
        out_shader->globalDescriptorSetLayout,
        out_shader->globalDescriptorSetLayout
    };

    VkDescriptorSetAllocateInfo alloc_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
    alloc_info.descriptorPool = out_shader->globalDescriptorPool;
    alloc_info.descriptorSetCount = 3;
    alloc_info.pSetLayouts = global_layouts;

    VK_CHECK(vkAllocateDescriptorSets(
        context->device.logical_device,
        &alloc_info,
        out_shader->globalDescriptorSets
    ));

    return true;
}

void vk_destroy_object_shader(vulkanContext* context, vkObjectShader* shader) {
    VkDevice logical_device = context->device.logical_device;

    // Destroy uniform buffer
    vulkanBuffer_destroy(context, &shader->globalUniformBuffer);

    // Destroy pipeline
    vk_destroy_graphics_pipeline(context, &shader->pipeline);

    // Destroy global descriptor pool
    vkDestroyDescriptorPool(logical_device, shader->globalDescriptorPool, context->allocator);

    // Destroy descriptor set layouts
    vkDestroyDescriptorSetLayout(logical_device, shader->globalDescriptorSetLayout, context->allocator);

    // Destroy shader modules
    for (i32 i = 0; i < OBJECT_SHADER_STAGE_COUNT; i++) {
        vkDestroyShaderModule(context->device.logical_device, shader->stages[i].handle, context->allocator);
        shader->stages[i].handle = NULL;
    }
}

void vk_use_object_shader(vulkanContext* context, vkObjectShader* shader) {
    u32 image_index = context->imageIndex;
    vk_bind_pipeline(&context->graphics_command_buffers[image_index], VK_PIPELINE_BIND_POINT_GRAPHICS, &shader->pipeline);
}


void vk_update_object_shader_global_state(vulkanContext* context, vkObjectShader* shader) {
    u32 image_index = context->imageIndex;
    VkCommandBuffer command_buffer = context->graphics_command_buffers[image_index].handle;
    VkDescriptorSet global_descriptor = shader->globalDescriptorSets[image_index];

    // TODO: FIX
    // Configure the descriptors for the given index.
    u32 range = sizeof(globalUniformObject);
    u64 offset = sizeof(globalUniformObject) * image_index;

    // Copy the data to buffer
    vulkanBuffer_load_data(context, &shader->globalUniformBuffer, offset, range, 0, &shader->global_ubo);

    VkDescriptorBufferInfo bufferInfo;
    bufferInfo.buffer = shader->globalUniformBuffer.handle;
    bufferInfo.offset = offset;
    bufferInfo.range = range;

    // Update descriptor sets.
    VkWriteDescriptorSet descriptor_write = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
    descriptor_write.dstSet = shader->globalDescriptorSets[image_index];
    descriptor_write.dstBinding = 0;
    descriptor_write.dstArrayElement = 0;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptor_write.descriptorCount = 1;
    descriptor_write.pBufferInfo = &bufferInfo;

    vkUpdateDescriptorSets(context->device.logical_device, 1, &descriptor_write, 0, NULL);
    // TODO: FIX'N

    // Bind the global descriptor set to be updated
    vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, shader->pipeline.pipelineLayout, 0, 1, &global_descriptor, 0, NULL);
}

void vk_update_object_shader(vulkanContext* context, vkObjectShader* shader, mat4 model) {
    u32 image_index = context->imageIndex;
    VkCommandBuffer command_buffer = context->graphics_command_buffers[image_index].handle;

    vkCmdPushConstants(command_buffer, shader->pipeline.pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(mat4), &model);
}