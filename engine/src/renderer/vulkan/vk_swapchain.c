#include "renderer/vulkan/vk_swapchain.h"

#include "renderer/vulkan/vk_device.h"
#include "renderer/vulkan/vk_image.h"

#include "core/logger.h"
#include "memory/hmemory.h"
#include "utils/utils.h"

// Internal methods
void create(
    vulkanContext* context,
    u32 width,
    u32 height,
    vulkanSwapchain* swapchain
);
void destroy(
    vulkanContext* context,
    vulkanSwapchain* swapchain
);

void createVulkanSwapchain(
    vulkanContext* context,
    u32 width,
    u32 height,
    vulkanSwapchain* outSwapchain) {

    // Just creates a swapchain.
    create(context, width, height, outSwapchain);
}

void recreateVulkanSwapchain(
    vulkanContext* context,
    u32 width,
    u32 height,
    vulkanSwapchain* swapchain) {

    // Destroy the old swapchain and create a new one.
    destroy(context, swapchain);
    create(context, width, height, swapchain);
}

void destroyVulkanSwapchain(
    vulkanContext* context,
    vulkanSwapchain* swapchain) {

    // Destroy the swapchain.
    destroy(context, swapchain);
}

b8 vk_swapchain_acquire_next_image_index(
    vulkanContext* context,
    vulkanSwapchain* swapchain,
    u64 timeout_ns,
    VkSemaphore image_aviable_semaphore,
    VkFence fence,
    u32 *out_image_index) {

    VkResult result = vkAcquireNextImageKHR(
        context->device.logical_device, 
        swapchain->handle,
        timeout_ns,
        image_aviable_semaphore,
        fence,
        out_image_index
    );

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        // Trigger swapchain recreation, then boot out of the render loop.
        recreateVulkanSwapchain(context, context->framebuffer_width, context->framebuffer_height, swapchain);
        return false;
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        HFATAL("Failed to acquire swapchain image!");
        return false;
    }

    return true;
}

void vk_swapchain_present(
    vulkanContext* context,
    vulkanSwapchain* swapchain,
    VkQueue graphics_queue,
    VkQueue present_queue,
    VkSemaphore render_complete_semaphore,
    u32 present_image_index) {

    // Return the image to the swapchain for presentation.
    VkPresentInfoKHR present_info = {VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = &render_complete_semaphore;
    present_info.swapchainCount = 1;
    present_info.pSwapchains = &swapchain->handle;
    present_info.pImageIndices = &present_image_index;
    present_info.pResults = NULL;

    VkResult result = vkQueuePresentKHR(present_queue, &present_info);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        // Swapchain is out of date, suboptimal or a framebuffer resize has occurred. Thrigger swapchain recreation.
        recreateVulkanSwapchain(context, context->framebuffer_width, context->framebuffer_height, swapchain);
    }
    else if (result != VK_SUCCESS) {
        HFATAL("Failed to present swapchain image!");   
    }

    // Increment (and loop) the index.
    context->curFrame = (context->curFrame + 1) % (swapchain->max_frames_in_flight);
}

void create(
    vulkanContext* context,
    u32 width,
    u32 height,
    vulkanSwapchain* swapchain) {
    
    VkExtent2D swapchainExtent = { width, height };

    // Choose a swap surface format.
    b8 found = false;
    for (u32 i = 0; i < context->device.swapchain_support.format_count; i++) {
        VkSurfaceFormatKHR format = context->device.swapchain_support.formats[i];
        // Prefered formats
        if (format.format == VK_FORMAT_B8G8R8A8_UNORM && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            swapchain->imageFormat = format;
            found = true;
            break;
        }
    } 

    if (!found) {
        swapchain->imageFormat = context->device.swapchain_support.formats[0];
    }

    VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR;
    for (u32 i = 0; i < context->device.swapchain_support.present_mode_count; i++) {
        VkPresentModeKHR mode = context->device.swapchain_support.present_modes[i];
        if (mode == VK_PRESENT_MODE_MAILBOX_KHR) {
            present_mode = mode;
            break;
        }
    }

    // Requery the swapchain support
    vulkanDeviceQuerySwapChainSupport(
        context->device.physical_device,
        context->surface,
        &context->device.swapchain_support
    );

    // Swapchain extent
    if (context->device.swapchain_support.capabilities.currentExtent.width != UINT32_MAX) {
        swapchainExtent = context->device.swapchain_support.capabilities.currentExtent;
    }

    // Clamp to the value allowed by the GPU
    VkExtent2D min = context->device.swapchain_support.capabilities.minImageExtent;
    VkExtent2D max = context->device.swapchain_support.capabilities.maxImageExtent;
    swapchainExtent.width = clamp(swapchainExtent.width, min.width, max.width);
    swapchainExtent.height = clamp(swapchainExtent.height, min.height, max.height);

    u32 imageCount = context->device.swapchain_support.capabilities.minImageCount + 1;
    if (context->device.swapchain_support.capabilities.maxImageCount > 0 && imageCount > context->device.swapchain_support.capabilities.maxImageCount) {
        imageCount = context->device.swapchain_support.capabilities.maxImageCount;
    }

    swapchain->max_frames_in_flight = imageCount - 1;

    // Swapchain create info (SCI)
    VkSwapchainCreateInfoKHR SCI = {VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};
    SCI.surface = context->surface;
    SCI.minImageCount = imageCount;
    SCI.imageFormat = swapchain->imageFormat.format;
    SCI.imageColorSpace = swapchain->imageFormat.colorSpace;
    SCI.imageExtent = swapchainExtent;
    SCI.imageArrayLayers = 1;
    SCI.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    // Setup the queue family indices
    if (context->device.graphics_queue_index != context->device.present_queue_index) {
        u32 queueFamilyIndices[] = { 
            (u32)context->device.graphics_queue_index, 
            (u32)context->device.present_queue_index 
        };
        SCI.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        SCI.queueFamilyIndexCount = 2;
        SCI.pQueueFamilyIndices = queueFamilyIndices;
    }
    else {
        SCI.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        SCI.queueFamilyIndexCount = 0;
        SCI.pQueueFamilyIndices = NULL;
    }
    
    SCI.preTransform = context->device.swapchain_support.capabilities.currentTransform;
    SCI.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    SCI.presentMode = present_mode;
    SCI.clipped = VK_TRUE;
    SCI.oldSwapchain = NULL;

    VK_CHECK(vkCreateSwapchainKHR(context->device.logical_device, &SCI, context->allocator, &swapchain->handle));

    // Start with a zero frame index.
    context->curFrame = 0;

    // Images
    swapchain->imageCount = 0;
    VK_CHECK(vkGetSwapchainImagesKHR(context->device.logical_device, swapchain->handle, &swapchain->imageCount, NULL));
    if (!swapchain->images) {
        swapchain->images = (VkImage*)Hallocate(sizeof(VkImage*) * swapchain->imageCount, MEMORY_TAG_RENDERER);
    }
    if (!swapchain->views) {
        swapchain->views = (VkImageView*)Hallocate(sizeof(VkImageView*) * swapchain->imageCount, MEMORY_TAG_RENDERER);
    }
    VK_CHECK(vkGetSwapchainImagesKHR(context->device.logical_device, swapchain->handle, &swapchain->imageCount, swapchain->images));

    // Views
    for (u32 i = 0; i < swapchain->imageCount; i++) {
        VkImageViewCreateInfo viewInfo = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
        viewInfo.image = swapchain->images[i];
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = swapchain->imageFormat.format;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        VK_CHECK(vkCreateImageView(context->device.logical_device, &viewInfo, context->allocator, &swapchain->views[i]));
    }

    // Depth resources
    if (!vulkanDeviceDetectDepthFormat(&context->device)) {
        context->device.depth_format = VK_FORMAT_UNDEFINED;
        HFATAL("Failed to find a supported format.");
    }

    // Create depth image and its view.
    createVulkanImage(
        context,
        VK_IMAGE_TYPE_2D,
        swapchainExtent.width,
        swapchainExtent.height,
        context->device.depth_format,
        VK_IMAGE_TILING_OPTIMAL, 
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        true,
        VK_IMAGE_ASPECT_DEPTH_BIT,
        &swapchain->depth_attachment
    );

    HINFO("Swapchain created successfully.");    
}

void destroy(
    vulkanContext* context,
    vulkanSwapchain* swapchain) {

    vkDeviceWaitIdle(context->device.logical_device);
    destroyVulkanImage(context, &swapchain->depth_attachment);

    // Only destroy the views, not the images, since those are owned by the swapchain and are thus
    // destroyed when it is.
    for (u32 i = 0; i < swapchain->imageCount; i++) {
        vkDestroyImageView(context->device.logical_device, swapchain->views[i], context->allocator);
    }

    vkDestroySwapchainKHR(context->device.logical_device, swapchain->handle, context->allocator);
}