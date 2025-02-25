#pragma once

#include "renderer/vulkan/vk_types.inl"

b8 create_shader_module(
    vulkanContext* context,
    const char* name,
    const char* typeStr,
    VkShaderStageFlagBits shaderStageFlag,
    u32 stageIndex,
    vkShaderStage* shaderStages
);