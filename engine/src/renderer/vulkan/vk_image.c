#include "renderer/vulkan/vk_image.h"

#include "renderer/vulkan/vk_device.h"

#include "memory/hmemory.h"
#include "core/logger.h"

void createVulkanImage(
    vulkanContext* context,
    VkImageType imageType,
    u32 width,
    u32 height,
    VkFormat format,
    VkImageTiling tiling,
    VkImageUsageFlags usage,
    VkMemoryPropertyFlags memory_flags,
    b32 create_view,
    VkImageAspectFlags view_aspect_flags,
    vulkanImage* output_image) {

    // Copy params
    output_image->width = width;
    output_image->height = height;

    // Creation info.
    // Image create info(ICI)
    VkImageCreateInfo ICI = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
    ICI.imageType = VK_IMAGE_TYPE_2D;
    ICI.extent.width = width;
    ICI.extent.height = height;
    ICI.extent.depth = 1;       // TODO: Support configurable depth.
    ICI.mipLevels = 4;          // TODO: Support mip mapping.
    ICI.arrayLayers = 1;        // TODO: Support number of layers in the image.
    ICI.format = format;
    ICI.tiling = tiling;
    ICI.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    ICI.usage = usage;
    ICI.samples = VK_SAMPLE_COUNT_1_BIT;            // TODO: Configurable sample count.
    ICI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;    // TODO: Configurable sharing mode.

    VK_CHECK(vkCreateImage(context->device.logical_device, &ICI, context->allocator, &output_image->handle));

    // Querry memory requirements.
    VkMemoryRequirements memoryRequirements;
    vkGetImageMemoryRequirements(context->device.logical_device, output_image->handle, &memoryRequirements);

    i32 memory_type = context->find_memory_index(memoryRequirements.memoryTypeBits, memory_flags);
    if (memory_type == -1) {
        HERROR("Required memory type not found. Image not valid.");
    }

    // Allocate memory
    VkMemoryAllocateInfo memoryAllocateInfo = {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
    memoryAllocateInfo.allocationSize = memoryRequirements.size;
    memoryAllocateInfo.memoryTypeIndex = memory_type;
    VK_CHECK(vkAllocateMemory(context->device.logical_device, &memoryAllocateInfo, context->allocator, &output_image->memory));

    // Bind the memory
    // TODO: Configurable memory offset.                                                                  *offset*
    VK_CHECK(vkBindImageMemory(context->device.logical_device, output_image->handle, output_image->memory, 0));

    // Create view
    if (create_view) {
        output_image->view = NULL;
        createVulkanImageView(context, format, output_image, view_aspect_flags);
    }
}

void createVulkanImageView(
    vulkanContext* context,
    VkFormat format,
    vulkanImage* image,
    VkImageAspectFlags aspect_flags) {

    // View create info(VCI)
    VkImageViewCreateInfo VCI = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
    VCI.image = image->handle;
    VCI.viewType = VK_IMAGE_VIEW_TYPE_2D;   // TODO: Make configurable.
    VCI.format = format;
    VCI.subresourceRange.aspectMask = aspect_flags;

    // TODO: Make configurable.
    VCI.subresourceRange.baseMipLevel = 0;
    VCI.subresourceRange.levelCount = 1;
    VCI.subresourceRange.baseArrayLayer = 0;
    VCI.subresourceRange.layerCount = 1;

    VK_CHECK(vkCreateImageView(context->device.logical_device, &VCI, context->allocator, &image->view));
}

void destroyVulkanImage(
    vulkanContext* context,
    vulkanImage* image) {

    if (image->view) {
        vkDestroyImageView(context->device.logical_device, image->view, context->allocator);
        image->view = NULL;
    }
    if (image->memory) {
        vkFreeMemory(context->device.logical_device, image->memory, context->allocator);
        image->memory = NULL;
    }
    if (image->handle) {
        vkDestroyImage(context->device.logical_device, image->handle, context->allocator);
        image->handle = NULL;
    }
}