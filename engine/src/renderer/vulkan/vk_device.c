#include "renderer/vulkan/vk_device.h"

#include "core/logger.h"
#include "core/hmemory.h"
#include "containers/darray.h"
#include "utils/hstring.h"

typedef struct vk_physical_device_requequirements {
    b8 graphics;
    b8 present;
    b8 compute;
    b8 transfer;
    // darray
    const char** device_extension_names;
    b8 sampler_anisotropy;
    b8 discrete_gpu;
} vk_physical_device_requequirements;

typedef struct vk_physical_device_queue_family_info {
    u32 graphics_family_index;
    u32 present_family_index;
    u32 compute_family_index;
    u32 transfer_family_index;
} vk_physical_device_queue_family_info;

b8 selectPhysicalDevice(vulkanContext* context);
b8 physicalDeviceMeetsRequirements(
    VkPhysicalDevice device, 
    VkSurfaceKHR surface, 
    const VkPhysicalDeviceProperties* properties,
    const VkPhysicalDeviceFeatures* features,
    const vk_physical_device_requequirements* requirements,
    vk_physical_device_queue_family_info* out_queue_info,
    vk_swapchain_support_info* out_swapchain_support
);

b8 createVulkanDevice(vulkanContext *context) {
    if (!selectPhysicalDevice(context)) {
        return false;
    }

    HINFO("Creating logical device...");

    // NOTE: Do not create additional queues for shared indices.
    b8 present_shares_graphics_queue = context->device.present_queue_index == context->device.graphics_queue_index;
    b8 transfer_shares_graphics_queue = context->device.transfer_queue_index == context->device.graphics_queue_index;
    u32 index_count = 1;
    
    // Change to if++ if this goes wrong
    index_count+= (!transfer_shares_graphics_queue) + (!present_shares_graphics_queue);
    u32 indices[32];
    u8 index = 0;

    indices[index++] = context->device.graphics_queue_index;

    if (!present_shares_graphics_queue) {
        indices[index++] = context->device.present_queue_index;
    }
    if (!transfer_shares_graphics_queue) {
        indices[index++] = context->device.transfer_queue_index;
    }

    VkDeviceQueueCreateInfo queue_create_infos[32];
    for(u32 i = 0; i < index_count; i++) {
        queue_create_infos[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_create_infos[i].queueFamilyIndex = indices[i];
        queue_create_infos[i].queueCount = 1;

        // TODO: Enable this for a future enchancement.
        //if (indices[i] == context->device.graphics_queue_index) {
        //    queue_create_infos[i].queueCount = 2;
        //}
        
        queue_create_infos[i].flags = 0;
        queue_create_infos[i].pNext = NULL;
        f32 queue_priority = 1.0f;
        queue_create_infos[i].pQueuePriorities = &queue_priority;
    }

    // Request device features
    // TODO: Should be config driven
    VkPhysicalDeviceFeatures device_features = {};
    device_features.samplerAnisotropy = VK_TRUE; // Request anisotropy
    
    VkDeviceCreateInfo device_create_info = {VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
    device_create_info.queueCreateInfoCount = index_count;
    device_create_info.pQueueCreateInfos = queue_create_infos;
    device_create_info.pEnabledFeatures = &device_features;
    device_create_info.enabledExtensionCount = 1;
    const char* extension_names = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
    device_create_info.ppEnabledExtensionNames = &extension_names;

    // Deprecated and ingnored, so pass nothing
    device_create_info.enabledLayerCount = 0;
    device_create_info.ppEnabledLayerNames = NULL;

    // Create the device
    VK_CHECK(vkCreateDevice(
        context->device.physical_device,
        &device_create_info,
        context->allocator,
        &context->device.logical_device
    ));
    HINFO("Logical device created.");

    // Get queues
    vkGetDeviceQueue(
        context->device.logical_device,
        context->device.graphics_queue_index,
        0, 
        &context->device.graphics_queue
    );
    vkGetDeviceQueue(
        context->device.logical_device,
        context->device.present_queue_index,
        0, 
        &context->device.present_queue
    );
    vkGetDeviceQueue(
        context->device.logical_device,
        context->device.transfer_queue_index,
        0, 
        &context->device.transfer_queue
    );
    HINFO("Queues obtained.");

    // Create command pool for graphics queue.
    VkCommandPoolCreateInfo pool_create_info = {VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
    pool_create_info.queueFamilyIndex = context->device.graphics_queue_index;
    pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    VK_CHECK(vkCreateCommandPool(
        context->device.logical_device,
        &pool_create_info,
        context->allocator,
        &context->device.graphics_command_pool
    ));
    HINFO("Graphics command pool created.");

    return true;
}

void destroyVulkanDevice(vulkanContext *context) {
    // Unset queues
    context->device.graphics_queue = NULL;
    context->device.present_queue = NULL;
    context->device.transfer_queue = NULL;

    HINFO("Destroying command pools...");
    vkDestroyCommandPool(
        context->device.logical_device, 
        context->device.graphics_command_pool,
        context->allocator
    );

    // Destroy logical device
    HINFO("Destroying logical device...");
    if (context->device.logical_device) {
        vkDestroyDevice(context->device.logical_device, context->allocator);
        context->device.logical_device = NULL;
    }

    // Release physical device resources (physical devices can't be destroyed)
    HINFO("Releasing physical device resources...");
    context->device.physical_device = NULL;

    if (context->device.swapchain_support.formats) {
        // TODO: change this to Hfree
        Hfree(
            context->device.swapchain_support.formats,
            sizeof(VkSurfaceFormatKHR) * context->device.swapchain_support.format_count,
            MEMORY_TAG_RENDERER
        );
        context->device.swapchain_support.formats = NULL;
        context->device.swapchain_support.format_count = 0;
    }

    if (context->device.swapchain_support.present_modes) {
        Hfree(
            context->device.swapchain_support.present_modes,
            sizeof(VkPresentModeKHR) * context->device.swapchain_support.present_mode_count,
            MEMORY_TAG_RENDERER
        );
        context->device.swapchain_support.present_modes = NULL;
        context->device.swapchain_support.present_mode_count = 0;
    }

    HzeroMemory(
        &context->device.swapchain_support.capabilities,
        sizeof(context->device.swapchain_support.capabilities)
    );

    context->device.graphics_queue_index = -1;
    context->device.present_queue_index = -1;
    context->device.transfer_queue_index = -1;
}

void vulkanDeviceQuerySwapChainSupport(
    VkPhysicalDevice physical_device,
    VkSurfaceKHR surface,
    vk_swapchain_support_info* out_support_info) {

    // Surface capabilities
    VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &out_support_info->capabilities));

    // Surface formats
    VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &out_support_info->format_count, NULL));
    if (out_support_info->format_count != 0) {
        if (!out_support_info->formats) {
            out_support_info->formats = Hallocate(sizeof(VkSurfaceFormatKHR) * out_support_info->format_count, MEMORY_TAG_RENDERER);
        }
        VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &out_support_info->format_count, out_support_info->formats));
    }

    // Present Modes
    VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &out_support_info->present_mode_count, NULL));
    if (out_support_info->present_mode_count != 0) {
        if (!out_support_info->present_modes) {
            out_support_info->present_modes = Hallocate(sizeof(VkPresentModeKHR) * out_support_info->present_mode_count, MEMORY_TAG_RENDERER);
        }
        VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &out_support_info->present_mode_count, out_support_info->present_modes));
    }
}

b8 vulkanDeviceDetectDepthFormat(vulkanDevice* device) {
    // Format candidates
    const u64 candidateCount = 3;
    VkFormat candidates[3] = {
        VK_FORMAT_D32_SFLOAT,
        VK_FORMAT_D32_SFLOAT_S8_UINT,
        VK_FORMAT_D24_UNORM_S8_UINT
    };

    u32 flags = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;
    for (u64 i = 0; i < candidateCount; i++) {
        VkFormatProperties properties;
        vkGetPhysicalDeviceFormatProperties(device->physical_device, candidates[i], &properties);

        if (((properties.linearTilingFeatures & flags) == flags) ||
            ((properties.optimalTilingFeatures & flags) == flags)) {
            device->depth_format = candidates[i];
            return true;
        }
        // Old format detection
        /*if ((properties.linearTilingFeatures & flags) == flags)  {
            device->depth_format = candidates[i];
            return true;
        }
        else if ((properties.optimalTilingFeatures & flags) == flags) {
            device->depth_format = candidates[i];
            return true;
        }*/
    }
    return false;
}

b8 selectPhysicalDevice(vulkanContext* context) {
    u32 physicalDeviceCount = 0;
    VK_CHECK(vkEnumeratePhysicalDevices(context->instance, &physicalDeviceCount, NULL));
    if (physicalDeviceCount == 0) {
        HFATAL("No devices which support Vulkan were found.");
        return false;
    }

    const u32 max_device_count = 32;
    VkPhysicalDevice physicalDevices[max_device_count];
    VK_CHECK(vkEnumeratePhysicalDevices(context->instance, &physicalDeviceCount, physicalDevices));
    for (u32 i = 0; i < physicalDeviceCount; i++) {
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(physicalDevices[i], &properties);

        VkPhysicalDeviceFeatures features;
        vkGetPhysicalDeviceFeatures(physicalDevices[i], &features);

        VkPhysicalDeviceMemoryProperties memory;
        vkGetPhysicalDeviceMemoryProperties(physicalDevices[i], &memory);

        // TODO: These requirements should probably be driven by engine
        // Configuration
        vk_physical_device_requequirements requirements = {};
        requirements.graphics = true;
        requirements.present = true;
        requirements.transfer = true;
        // NOTE: Enable this if compute will be required
        // requirements.compute = true;
        requirements.sampler_anisotropy = true;
        requirements.discrete_gpu = false;
        requirements.device_extension_names = darray_create(const char*);
        darray_push(requirements.device_extension_names, &VK_KHR_SWAPCHAIN_EXTENSION_NAME);

        vk_physical_device_queue_family_info queue_info = {};
        b8 result = physicalDeviceMeetsRequirements(
            physicalDevices[i], 
            context->surface, 
            &properties, 
            &features, 
            &requirements, 
            &queue_info, 
            &context->device.swapchain_support
        );

        if (result) {
            HINFO("Selected device: %s.", properties.deviceName);
            // GPU type, etc. 
            switch (properties.deviceType) {
                default:
                case VK_PHYSICAL_DEVICE_TYPE_OTHER:
                    HINFO("GPU type is Unknown");
                    break;
                case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
                    HINFO("GPU type is Integrated");
                    break;
                case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
                    HINFO("GPU type is Descrete");
                    break;
                case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
                    HINFO("GPU type is Virtual");
                    break;
                case VK_PHYSICAL_DEVICE_TYPE_CPU:
                    HINFO("GPU type is CPU");
                    break;
            }

            // GPU Driver Version
            HINFO(
                "GPU Driver Version: %d.%d.%d",
                VK_VERSION_MAJOR(properties.driverVersion), 
                VK_VERSION_MINOR(properties.driverVersion), 
                VK_VERSION_PATCH(properties.driverVersion)
            );
            // Vulkan API Version
            HINFO(
                "Vulkan API Version: %d.%d.%d",
                VK_VERSION_MAJOR(properties.apiVersion), 
                VK_VERSION_MINOR(properties.apiVersion), 
                VK_VERSION_PATCH(properties.apiVersion)
            );
            // Memory Information
            for (u32 j = 0; j < memory.memoryHeapCount; j++) {
                f32 memory_size_gib = (((f32)memory.memoryHeaps[j].size) / 1024.0f / 1024.0f / 1024.0f);

                // Dedicated GPU?
                if (memory.memoryHeaps[j].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) {
                    HINFO("Local GPU memory: %.2f GiB", memory_size_gib);
                }
                else {
                    HINFO("Shared System memory: %.2f GiB", memory_size_gib);
                }
            }

            context->device.physical_device = physicalDevices[i];
            context->device.graphics_queue_index = queue_info.graphics_family_index;
            context->device.present_queue_index = queue_info.present_family_index;
            context->device.transfer_queue_index = queue_info.transfer_family_index;
            // NOTE: Set compute index here if needed

            // Keep a copy of properties, features and memory info for later use
            context->device.properties = properties;
            context->device.features = features;
            context->device.memory = memory;
            break;
        }
    }

    // Ensure that a device was selected
    if (!context->device.physical_device) {
        HERROR("No physical devices were found which meet the requirements.");
        return false;
    }

    HINFO("Physical device selected.");
    return true;
}

b8 physicalDeviceMeetsRequirements(
    VkPhysicalDevice device, 
    VkSurfaceKHR surface, 
    const VkPhysicalDeviceProperties* properties,
    const VkPhysicalDeviceFeatures* features,
    const vk_physical_device_requequirements* requirements,
    vk_physical_device_queue_family_info* out_queue_info,
    vk_swapchain_support_info* out_swapchain_support) {

    // Evaluate device properties to determine if it meets the needs of our application
    out_queue_info->graphics_family_index = -1;
    out_queue_info->present_family_index = -1;
    out_queue_info->compute_family_index = -1;
    out_queue_info->transfer_family_index = -1;

    // Discrete GPU?
    if (requirements->discrete_gpu) {
        if (properties->deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
            HINFO("Device is not a discrete GPU, and one is required. Skipping...");
            return false;
        }
    }

    u32 queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, NULL);
    VkQueueFamilyProperties queue_families[32];
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_families);

    // Look at each queue and see what queues it supports
    HINFO("Graphics | Present | Compute | Transfer | Name");
    u8 min_transfer_score = 255;
    for (u32 i = 0; i < queue_family_count; i++) {
        u8 cur_transfer_score = 0;

        // Graphics queue?
        if(queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            out_queue_info->graphics_family_index = i;
            cur_transfer_score++;
        }

        // Compute queue?
        if(queue_families[i].queueFlags & VK_QUEUE_COMPUTE_BIT) {
            out_queue_info->compute_family_index = i;
            cur_transfer_score++;
        }

        // Transfer queue?
        if(queue_families[i].queueFlags & VK_QUEUE_TRANSFER_BIT) {
            // Take the index. If it's the current lowest. This increases the
            // linklihood if it's a dedicated transfer queue.
            if (cur_transfer_score <= min_transfer_score) {
                min_transfer_score = cur_transfer_score;
                out_queue_info->transfer_family_index = i;
            }
        }

        // Present queue?
        VkBool32 supports_present = VK_FALSE;
        VK_CHECK(vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &supports_present));
        if (supports_present) {
            out_queue_info->present_family_index = i;
        } 
    }

    // Print out some info about the device
    HINFO("       %d |       %d |       %d |        %d | %s",
           out_queue_info->graphics_family_index != 1,
           out_queue_info->present_family_index != 1,
           out_queue_info->compute_family_index != 1,
           out_queue_info->transfer_family_index != 1,
           properties->deviceName
    );

    if (
        (!requirements->graphics || (requirements->graphics && out_queue_info->graphics_family_index != -1)) &&
        (!requirements->present || (requirements->present && out_queue_info->present_family_index != -1)) &&
        (!requirements->compute || (requirements->compute && out_queue_info->compute_family_index != -1)) &&
        (!requirements->transfer || (requirements->transfer && out_queue_info->transfer_family_index != -1))) {

        HINFO("Device meets queue requirements.");
        HTRACE("Graphics Family Index: %i", out_queue_info->compute_family_index);
        HTRACE("Present  Family Index: %i", out_queue_info->present_family_index);
        HTRACE("Transfer Family Index: %i", out_queue_info->transfer_family_index);
        HTRACE("Compute  Family Index: %i", out_queue_info->compute_family_index);

        //Query swapchain support
        vulkanDeviceQuerySwapChainSupport(device, surface, out_swapchain_support);

        if (out_swapchain_support->format_count < 1 || out_swapchain_support->present_mode_count < 1) {
            if (out_swapchain_support->formats) {
                Hfree(out_swapchain_support->formats, sizeof(VkSurfaceFormatKHR)* out_swapchain_support->format_count, MEMORY_TAG_RENDERER);
            }
            if (out_swapchain_support->present_modes) {
                Hfree(out_swapchain_support->present_modes, sizeof(VkPresentModeKHR)* out_swapchain_support->present_mode_count, MEMORY_TAG_RENDERER);
            }
            return false;
        }

        // Device Extensions
        if (requirements->device_extension_names) {
            u32 aviable_extension_count = 0;
            VkExtensionProperties* aviable_extensions = NULL;

            VK_CHECK(vkEnumerateDeviceExtensionProperties(device, NULL, &aviable_extension_count, NULL));
            if (aviable_extension_count != 0) {
                aviable_extensions = Hallocate(sizeof(VkExtensionProperties) * aviable_extension_count, MEMORY_TAG_RENDERER);
                VK_CHECK(vkEnumerateDeviceExtensionProperties(device, NULL, &aviable_extension_count, aviable_extensions));
            
                u32 required_extension_count = darray_length(requirements->device_extension_names);
                for (u32 i = 0; i < required_extension_count; i++) {
                    b8 found = false;
                    for (u32 j = 0; j < aviable_extension_count; j++) {
                        if (strings_equal(requirements->device_extension_names[i], aviable_extensions[j].extensionName)) {
                            found = true;
                            break;
                        }
                    }

                    if (!found) {
                        HINFO("Required extension not found: %s, skipping device...", requirements->device_extension_names[i]);
                        Hfree(aviable_extensions, sizeof(VkExtensionProperties) * aviable_extension_count, MEMORY_TAG_RENDERER);
                        return false;
                    }
                }
            }
            Hfree(aviable_extensions, sizeof(VkExtensionProperties) * aviable_extension_count, MEMORY_TAG_RENDERER);
        }

        // Sampler anisotropy
        if (requirements->sampler_anisotropy && !features->samplerAnisotropy) {
            HINFO("Device does not support samplerAnisotropy, skipping...");
            return false;
        }

        // Device meets all requirements
        return true;
    }

    // Device canot be used
    return false;
}