#pragma once

#include "renderer/vulkan/vk_types.inl"
#include "renderer/types.inl"

b8 vk_create_object_shader(vulkanContext* context, vkObjectShader* shader);

void vk_destroy_object_shader(vulkanContext* context, vkObjectShader* shader);

void vk_use_object_shader(vulkanContext* context, vkObjectShader* shader);

void vk_update_object_shader_global_state(vulkanContext* context, vkObjectShader* shader);

void vk_update_object_shader(vulkanContext* context, vkObjectShader* shader, mat4 model);