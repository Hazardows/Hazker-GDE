#include "renderer/vulkan/utils/vk_shader_utils.h"

#include "utils/hstring.h"
#include "core/logger.h"
#include "memory/hmemory.h"

#include "platform/filesystem.h"

static VkAllocationCallbacks* allocShader;

b8 create_shader_module(
    vulkanContext* context,
    const char* name,
    const char* typeStr,
    VkShaderStageFlagBits shaderStageFlag,
    u32 stageIndex,
    vkShaderStage* shaderStages) {

    // Build file name
    char filename[512];
    string_format(filename, "assets/shaders/%s.%s.spv", name, typeStr);

    HzeroMemory(&shaderStages[stageIndex].createInfo, sizeof(VkShaderModuleCreateInfo));
    shaderStages[stageIndex].createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    
    // Obtain file handle
    fileHandle handle;
    if (!filesystem_open(filename, FILE_MODE_READ, true, &handle)) {
        HERROR("Unable to read shader module: %s.", filename);
        return false;
    }

    // Read the entire file as binary
    u64 size = 0;
    u8* fileBuffer = NULL;
    if (!filesystem_read_all_bytes(&handle, &fileBuffer, &size)) {
        HERROR("Unable to binary read shader module: %s.", filename);
        return false;
    }
    shaderStages[stageIndex].createInfo.codeSize = size;
    shaderStages[stageIndex].createInfo.pCode = (u32*)fileBuffer;

    // Close the file
    filesystem_close(&handle);

    VK_CHECK(vkCreateShaderModule(
        context->device.logical_device,
        &shaderStages[stageIndex].createInfo,
        allocShader,
        &shaderStages[stageIndex].handle
    ));

    // Shader stage info
    HzeroMemory(&shaderStages[stageIndex].shaderStageCreateInfo, sizeof(VkPipelineShaderStageCreateInfo));
    shaderStages[stageIndex].shaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[stageIndex].shaderStageCreateInfo.stage = shaderStageFlag;
    shaderStages[stageIndex].shaderStageCreateInfo.module = shaderStages[stageIndex].handle;
    shaderStages[stageIndex].shaderStageCreateInfo.pName = "main";

    if (fileBuffer) {
        Hfree(fileBuffer, sizeof(u8) * size, MEMORY_TAG_STRING);
        fileBuffer = NULL;
    }

    return true;
}