#include "renderer/vulkan/vk_fence.h"

#include "core/logger.h"

void createVulkanFence(
    vulkanContext* context, 
    b8 createSignaled,
    vulkanFence* outFence) {

    // Make sure to signal the fence if required
    outFence->isSignaled = createSignaled;
    VkFenceCreateInfo fence_create_info = {VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
    if (outFence->isSignaled) {
        fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    }

    VK_CHECK(vkCreateFence(
        context->device.logical_device,
        &fence_create_info,
        context->allocator,
        &outFence->handle
    ));
}

void destroyVulkanFence(vulkanContext* context, vulkanFence* fence) {
    if (fence->handle) {
        vkDestroyFence(
            context->device.logical_device,
            fence->handle,
            context->allocator
        );
        fence->handle = NULL;
    }
    fence->isSignaled = false;
}

b8 vulkanFenceWait(vulkanContext* context, vulkanFence* fence, u64 timeout_ns) {
    if (!fence->isSignaled) {
        VkResult result = vkWaitForFences(
            context->device.logical_device,
            1,
            &fence->handle,
            true,
            timeout_ns
        );

        switch (result) {
            case VK_SUCCESS:
                fence->isSignaled = true;
                return true;
            case VK_TIMEOUT:
                HWARNING("vkFenceWait - Timed out");
                break;
            case VK_ERROR_DEVICE_LOST:
                HERROR("vkFenceWait - VK_ERROR_DEVICE_LOST");
                break;
            case VK_ERROR_OUT_OF_HOST_MEMORY:
                HERROR("vkFenceWait - VK_ERROR_OUT_OF_HOST_MEMORY");
                break;
            case VK_ERROR_OUT_OF_DEVICE_MEMORY:
                HERROR("vkFenceWait - VK_ERROR_OUT_OF_DEVICE_MEMORY");
                break;
            default:
                HERROR("vkFenceWait - Unknown error");
                break;
        }
    }
    else {
        // If already signaled, do not wait.
        return true;
    }

    // Failure case
    return false;
}

void vulkanFenceReset(vulkanContext* context, vulkanFence* fence) {
    if (fence->isSignaled) {
        VK_CHECK(vkResetFences(context->device.logical_device, 1, &fence->handle));
        fence->isSignaled = false;
    }
}