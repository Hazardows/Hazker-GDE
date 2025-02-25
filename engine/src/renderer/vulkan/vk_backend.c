#include "renderer/vulkan/vk_backend.h"

#include "renderer/vulkan/vk_types.inl"
#include "renderer/vulkan/vk_platform.h"
#include "renderer/vulkan/vk_device.h"
#include "renderer/vulkan/vk_swapchain.h"
#include "renderer/vulkan/vk_renderpass.h"
#include "renderer/vulkan/vk_command_buffer.h"
#include "renderer/vulkan/vk_framebuffer.h"
#include "renderer/vulkan/vk_fence.h"
#include "renderer/vulkan/vk_buffer.h"

#include "core/logger.h"
#include "utils/hstring.h"
#include "containers/darray.h"
#include "memory/hmemory.h"
#include "core/app.h"

#include "utils/vk_utils.h"
#include "platform/platform.h"
#include "math/math_types.inl"

// Shaders
#include "renderer/vulkan/shaders/vk_object_shader.h"

// Static Vulkan context
static vulkanContext context;
static u32 cached_framebuffer_width = 0;
static u32 cached_framebuffer_height = 0;

VKAPI_ATTR VkBool32 VKAPI_CALL vk_debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
    VkDebugUtilsMessageTypeFlagsEXT message_types,
    const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
    void* user_data
);

i32 find_memory_index(u32 type_filter, u32 property_flags);
b8 create_buffers(vulkanContext* context);
void create_command_buffers(rendererBackend* backend);
void regenerate_framebuffers(rendererBackend* backend, vulkanSwapchain* swapchain, vulkanRenderPass* renderpass);
b8 recreateSwapchain(rendererBackend* backend);

void upload_data_range(vulkanContext* context, VkCommandPool pool, VkFence fence, VkQueue queue, vulkanBuffer* buffer, u64 offset, u64 size, void* data) {
    // Create a host-visible staging buffer to upload to. Mark it as the source of the transfer.
    VkBufferUsageFlags flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    vulkanBuffer staging;
    vulkanBuffer_create(context, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, flags, true, &staging);

    // Load the data into the staging buffer.
    vulkanBuffer_load_data(context, &staging, 0, size, 0, data);

    // Perform the copy from the staging buffer to the device local buffer.
    vulkanBuffer_copy_to(context, pool, fence, queue, staging.handle, 0, buffer->handle, offset, size);

    // Clean up the staging buffer.
    vulkanBuffer_destroy(context, &staging);
}

b8 vk_renderer_backend_init(struct rendererBackend* backend, const char* app_name) {
    // Function pointers
    context.find_memory_index = find_memory_index;
    
    // TODO: Custom allocator
    context.allocator = NULL;

    appGetFramebufferSize(&cached_framebuffer_width, &cached_framebuffer_height);
    context.framebuffer_width = (cached_framebuffer_width != 0) ? cached_framebuffer_width : 720;
    context.framebuffer_height = (cached_framebuffer_height != 0) ? cached_framebuffer_height : 480;
    cached_framebuffer_width = 0;
    cached_framebuffer_height = 0;
    
    // Setup Vulkan instance
    VkApplicationInfo app_info = {VK_STRUCTURE_TYPE_APPLICATION_INFO};
    app_info.apiVersion = VK_API_VERSION_1_2;
    app_info.pApplicationName = app_name;
    app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.pEngineName = "Hazker Engine";
    app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);

    VkInstanceCreateInfo createInfo = {VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
    createInfo.pApplicationInfo = &app_info;

    // Obtain a list of required extensions
    const char** reqExts = darray_create(const char*);
    darray_push(reqExts, &VK_KHR_SURFACE_EXTENSION_NAME);   // Generic surface extension
    platformGetRequiredExtensionNames(&reqExts);            // Platform specific extensions
    
    #if defined(_DEBUG)
        // Debug extension and utilities for debugging
        darray_push(reqExts, &VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

        HDEBUG("Required extensions:");
        u32 len = darray_length(reqExts);
        for (u32 i = 0; i < len; i++) {
            HDEBUG(reqExts[i]);
        }
    #endif

    createInfo.enabledExtensionCount = darray_length(reqExts);
    createInfo.ppEnabledExtensionNames = reqExts;

    // Validation layers
    const char** reqValayNames = NULL;
    u32 reqValayCount = 0;

    //* If validation should be done, get a list of the required validation layer names          *//
    //* and make sure they exist. Validation layers should only be enabled on non-release builds *//
    #if defined(_DEBUG) 
        HINFO("Validation layers enabled. Enumerating...");

        // The list of validation layers required
        reqValayNames = darray_create(const char*);
        darray_push(reqValayNames, &"VK_LAYER_KHRONOS_validation");
        reqValayCount = darray_length(reqValayNames);

        // Obtain a list of available validation layers
        u32 alayCount = 0;
        VK_CHECK(vkEnumerateInstanceLayerProperties(&alayCount, NULL));
        VkLayerProperties* alays = darray_reserve(VkLayerProperties, alayCount);
        VK_CHECK(vkEnumerateInstanceLayerProperties(&alayCount, alays));

        // Verify that all required layers are avialable
        for (u32 i = 0; i < reqValayCount; i++) {
            HINFO("Searching for layer %s...", reqValayNames[i]);
            b8 found = false;
            for (u32 j = 0; j < alayCount; j++) {
                if (strings_equal(reqValayNames[i], alays[j].layerName)) {
                    found = true;
                    HINFO("Found layer.");
                    break;
                }
            }

            if (!found) {
                HFATAL("Required validation layer is missing: %s", reqValayNames[i]);
                return false;
            }
        }
        HINFO("All required validation layers are present.");
    #endif

    createInfo.enabledLayerCount = reqValayCount;
    createInfo.ppEnabledLayerNames = reqValayNames;

    VK_CHECK(vkCreateInstance(&createInfo, context.allocator, &context.instance));
    HINFO("Vulkan instance created.");

    // Debugger
    #if defined(_DEBUG) 
        HDEBUG("Creating Vulkan debugger...");
        u32 log_severity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT
                         | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
                         | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;
                      // | VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;

        VkDebugUtilsMessengerCreateInfoEXT debug_create_info = {VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT};
        debug_create_info.messageSeverity = log_severity;
        debug_create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
        debug_create_info.pfnUserCallback = vk_debug_callback;

        PFN_vkCreateDebugUtilsMessengerEXT func = 
            (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(context.instance, "vkCreateDebugUtilsMessengerEXT");
        HASSERT_MSG(func, "Failed to create debug messenger!");
        VK_CHECK(func(context.instance, &debug_create_info, context.allocator, &context.debug_messenger));
        HDEBUG("Vulkan debugger created.");
    #endif

    // Surface creation
    HDEBUG("Creating Vulkan surface...");
    if (!platformCreateVulkanSurface(&context)) {
        HERROR("Failed to create Vulkan surface!");
        return false;
    }
    HDEBUG("Vulkan surface created.");

    // Device creation
    if (!createVulkanDevice(&context)) {
        HERROR("Failed to create Vulkan device.");
        return false;
    }

    // Swapchain
    createVulkanSwapchain(
        &context,
        context.framebuffer_width,
        context.framebuffer_height,
        &context.swapchain
    );

    createVulkanRenderpass(
        &context,
        &context.main_renderpass,
        0, 0, context.framebuffer_width, context.framebuffer_height,
        0.67f, 0.13f, 0.87f, 1.0f,  // Purple screen UwU
        1.0f,
        0
    );

    // Swapchain framebuffers.
    context.swapchain.framebuffers = darray_reserve(vulkanFramebuffer, context.swapchain.imageCount);
    regenerate_framebuffers(backend, &context.swapchain, &context.main_renderpass);

    // Create command buffers
    create_command_buffers(backend);

    // Create sync objects.
    context.imageAvailableSemaphores = darray_reserve(VkSemaphore, context.swapchain.max_frames_in_flight);
    context.queueCompleteSemaphores = darray_reserve(VkSemaphore, context.swapchain.max_frames_in_flight);
    context.inFlightFences = darray_reserve(vulkanFence, context.swapchain.max_frames_in_flight);

    for (u8 i = 0; i < context.swapchain.max_frames_in_flight; i++) {
        VkSemaphoreCreateInfo semaphore_create_info = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
        vkCreateSemaphore(context.device.logical_device, &semaphore_create_info, context.allocator, &context.imageAvailableSemaphores[i]);
        vkCreateSemaphore(context.device.logical_device, &semaphore_create_info, context.allocator, &context.queueCompleteSemaphores[i]);

        // Create the fence in a signaled state, indicating that the first frame has alredy been "rendered".
        // This will prevent the application from waiting indefinitely for the first frame to render since it
        // cannot be rendered until a frame is "rendered" before it.
        createVulkanFence(&context, true, &context.inFlightFences[i]);
    }

    // In flight fences should not yet exist at this point, so clear the list. These are stored in pointers
    // because the initial state should be 0, and will be 0 when not in use. Actual fences are not owned
    // by this list.
    context.imagesInFlight = darray_reserve(vulkanFence, context.swapchain.imageCount);
    for (u32 i = 0; i < context.swapchain.imageCount; i++) {
        context.imagesInFlight[i] = NULL;
    }

    if (!vk_create_object_shader(&context, &context.objectShader)) {
        HERROR("Error loading built-in basic_lighting shader");
        return false;
    }

    create_buffers(&context);

    // TODO: tomporary test code
        #define vert_count 4
        vertex_3d verts[vert_count];
        HzeroMemory(verts, sizeof(vertex_3d) * vert_count);

        const f32 f = 10.0f;

        verts[0].position.x = -0.5 * f;
        verts[0].position.y = -0.5 * f;

        verts[1].position.x = 0.5 * f;
        verts[1].position.y = 0.5 * f;

        verts[2].position.x = -0.5 * f;
        verts[2].position.y = 0.5 * f;

        verts[3].position.x = 0.5 * f;
        verts[3].position.y = -0.5 * f;

        const u32 index_count = 6;
        u32 indices[] = {0, 1, 2, 0, 3, 1};

        upload_data_range(&context, context.device.graphics_command_pool, NULL, context.device.graphics_queue, &context.object_vertex_buffer, 0, sizeof(vertex_3d) * vert_count, verts);
        upload_data_range(&context, context.device.graphics_command_pool, NULL, context.device.graphics_queue, &context.object_index_buffer, 0, sizeof(u32) * index_count, indices);
    // TODO: end temporary test code

    HINFO("Vulkan renderer initialized successfully.");
    return true;
}

void vk_renderer_backend_shutdown(struct rendererBackend* backend) { 
    vkDeviceWaitIdle(context.device.logical_device);

    // Destroy in the oposite order of creation.
    
    // Destroy buffers
    vulkanBuffer_destroy(&context, &context.object_vertex_buffer);
    vulkanBuffer_destroy(&context, &context.object_index_buffer);

    vk_destroy_object_shader(&context, &context.objectShader);

    for (u8 i = 0; i < context.swapchain.max_frames_in_flight; i++) {
        if (context.imageAvailableSemaphores[i]) {
            vkDestroySemaphore(
                context.device.logical_device, 
                context.imageAvailableSemaphores[i],
                context.allocator
            );
            context.imageAvailableSemaphores[i] = NULL;
        }

        if (context.queueCompleteSemaphores[i]) {
            vkDestroySemaphore(
                context.device.logical_device,
                context.queueCompleteSemaphores[i],
                context.allocator
            );
            context.queueCompleteSemaphores[i] = NULL;
        }
        destroyVulkanFence(&context, &context.inFlightFences[i]);
    }

    darray_destroy(context.imageAvailableSemaphores);
    context.imageAvailableSemaphores = NULL;
    
    darray_destroy(context.queueCompleteSemaphores);
    context.queueCompleteSemaphores = NULL;

    darray_destroy(context.inFlightFences);
    context.inFlightFences = NULL;

    darray_destroy(context.imagesInFlight);
    context.imagesInFlight = NULL;

    // Command buffers
    for (u32 i = 0; i < context.swapchain.imageCount; i++) {
        if (context.graphics_command_buffers[i].handle) {
            vulkanCommandBufferFree(
                &context,
                context.device.graphics_command_pool,
                &context.graphics_command_buffers[i]
            );
            context.graphics_command_buffers[i].handle = NULL;
        }
    }
    darray_destroy(context.graphics_command_buffers);
    context.graphics_command_buffers = NULL;

    // Framebuffers
    for (u32 i = 0; i < context.swapchain.imageCount; i++) {
        vulkanFramebufferDestroy(&context, &context.swapchain.framebuffers[i]);
    }

    // Renderpass
    destroyVulkanRenderpass(&context, &context.main_renderpass);

    // Swapchain
    destroyVulkanSwapchain(&context, &context.swapchain);

    HDEBUG("Destroying Vulkan device...");
    destroyVulkanDevice(&context);

    HDEBUG("Destroying Vulkan surface...");
    if (context.surface) {
        vkDestroySurfaceKHR(context.instance ,context.surface, context.allocator);
        context.surface = NULL;
    }

    HDEBUG("Destroying Vulkan debugger...");
#if defined(_DEBUG)
    if (context.debug_messenger) {
        PFN_vkDestroyDebugUtilsMessengerEXT func = 
            (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(context.instance, "vkDestroyDebugUtilsMessengerEXT");
        func(context.instance, context.debug_messenger, context.allocator);
    }
#endif

    HDEBUG("Destroying Vulkan Instance...");
    vkDestroyInstance(context.instance, context.allocator);
}

void vk_renderer_backend_on_resized(rendererBackend* backend, u16 width, u16 height) {
    // Updates the frame_buffer_size_generation, a counter which indicates
    // when the frame buffer size has been updated.
    cached_framebuffer_width = width;
    cached_framebuffer_height = height;
    context.framebuffer_size_generation++;

    HINFO("Vulkan renderer backend->resized w/h/gen: %i/%i/%llu", width, height, context.framebuffer_size_generation);
}

b8 vk_renderer_backend_begin_frame(struct rendererBackend* backend, f32 delta_t) {
    vulkanDevice* device = &context.device;

    // Check if recreating swap chain and boot out.
    if (context.recreating_swapchain) {
        VkResult result =   vkDeviceWaitIdle(device->logical_device);
        if (!vkr_isSuccess(result)) {
            HERROR("vk_render_backend_begin_frame vKDeviceWaitIdle (1) failed: '%s'", vkr_toString(result, true));
            return false;
        }
        HINFO("Recreating swapchain, booting.");
        return false;
    }

    if (context.framebuffer_size_generation != context.framebuffer_last_size_generation) {
        VkResult result =   vkDeviceWaitIdle(device->logical_device);
        if (!vkr_isSuccess(result)) {
            HERROR("vk_render_backend_begin_frame vKDeviceWaitIdle (2) failed: '%s'", vkr_toString(result, true));
            return false;
        }

        // If the swapchain recreation failed (because, idk, window was minimized),
        // boot out before unsetting the flag.
        if (!recreateSwapchain(backend)) {
            return false;
        }

        HINFO("Resized, booting.");
        return false;
    }

    // Wait for the execution of the current frame to complete. 
    // The fence being free will allow this one to move on.
    if (!vulkanFenceWait(
        &context, 
        &context.inFlightFences[context.curFrame],
        UINT64_MAX
    )) {
        HWARNING("In-flight fence wait failure!");
        return false;
    }

    // Acquire the next image from the swap chain.
    // Pass along the semaphore that should be signaled when this completes.
    // This same semaphore will later be waited on by the queue submission
    // to ensure this image is aviable.
    if (!vk_swapchain_acquire_next_image_index(
        &context,
        &context.swapchain,
        UINT64_MAX,
        context.imageAvailableSemaphores[context.curFrame],
        NULL,
        &context.imageIndex
    )) {
        return false;
    }

    // Begin recording commands.
    vulkanCommandBuffer* commandBuffer = &context.graphics_command_buffers[context.imageIndex];
    vulkanCommandBufferReset(commandBuffer);
    vulkanCommandBufferBegin(commandBuffer, false, false, false);

    // Dynamic state
    VkViewport viewport;
    viewport.x = 0.0f;
    viewport.y = (f32)context.framebuffer_height;
    viewport.width = (f32)context.framebuffer_width;
    viewport.height = -(f32)context.framebuffer_height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    // Scissor
    VkRect2D scissor;
    scissor.offset.x = scissor.offset.y = 0;
    scissor.extent.width = context.framebuffer_width;
    scissor.extent.height = context.framebuffer_height;

    vkCmdSetViewport(commandBuffer->handle, 0, 1, &viewport);
    vkCmdSetScissor(commandBuffer->handle, 0, 1, &scissor);

    context.main_renderpass.w = context.framebuffer_width;
    context.main_renderpass.h = context.framebuffer_height;

    // Begin the renderpass
    vulkanRenderPassBegin(
        commandBuffer,
        &context.main_renderpass,
        context.swapchain.framebuffers[context.imageIndex].handle
    );

    return true;
}

void vk_renderer_update_global_state(mat4 projection, mat4 view, vec3 view_position, vec4 ambient_colour, i32 mode) {    
    vk_use_object_shader(&context, &context.objectShader);

    context.objectShader.global_ubo.projection = projection;
    context.objectShader.global_ubo.view = view;

    // TODO: other ubo properties

    vk_update_object_shader_global_state(&context, &context.objectShader);
}

b8 vk_renderer_backend_end_frame(struct rendererBackend* backend, f32 delta_t) {
    vulkanCommandBuffer* commandBuffer = &context.graphics_command_buffers[context.imageIndex];
    
    // End renderpass
    vulkanRenderPassEnd(commandBuffer, &context.main_renderpass);
    vulkanCommandBufferEnd(commandBuffer);

    // Make sure the previous frame is not using this image (i.e. Its fence is being waited on)
    if (context.imagesInFlight[context.imageIndex] != VK_NULL_HANDLE) { // was frame
        vulkanFenceWait(
            &context,
            context.imagesInFlight[context.imageIndex],
            UINT64_MAX
        );
    }

    // Mark the image fence as in-use by this frame.
    context.imagesInFlight[context.imageIndex] = &context.inFlightFences[context.curFrame];

    // Reset the fence for use on the next frame
    vulkanFenceReset(&context, &context.inFlightFences[context.curFrame]);

    // Submit the queue and wait for the operation to complete.
    // Begin queue submission
    VkSubmitInfo submitInfo = {VK_STRUCTURE_TYPE_SUBMIT_INFO};

    // Command buffer(s) to be executed.
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer->handle;

    // The semaphores(s) to be signaled when the queue is complete.
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &context.queueCompleteSemaphores[context.curFrame];

    // Wait semaphore ensures that the operation cannot begin until the image is aviable.
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &context.imageAvailableSemaphores[context.curFrame];

    // Each semaphore waits on the corresponding pipeline stage to complete. 1:1 ratio.
    // VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT prevents subsequent colour attachment
    // writes from executing until the semaphore signals (i.e. one frame is presented at a time)
    VkPipelineStageFlags flags[1] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.pWaitDstStageMask = flags;

    VkResult result = vkQueueSubmit(
        context.device.graphics_queue, 
        1,
        &submitInfo,
        context.inFlightFences[context.curFrame].handle
    );
    if (result != VK_SUCCESS) {
        HERROR("vkQueueSubmit failed with result: %s", vkr_toString(result, true));
        return false;
    }

    vulkanCommandBufferUpdateSubmited(commandBuffer);
    // End queue submission

    // Give the image back to the swapchain.
    vk_swapchain_present(
        &context,
        &context.swapchain,
        context.device.graphics_queue,
        context.device.present_queue,
        context.queueCompleteSemaphores[context.curFrame],
        context.imageIndex
    );

    return true;
}


void vk_backend_update_object(mat4 model) {
    vk_update_object_shader(&context, &context.objectShader, model);

    // TODO: temporary test code
        vulkanCommandBuffer* commandBuffer = &context.graphics_command_buffers[context.imageIndex];
        vk_use_object_shader(&context, &context.objectShader);

        // Bind vertex buffer at offset.
        VkDeviceSize offsets[1] = {0};
        vkCmdBindVertexBuffers(commandBuffer->handle, 0, 1, &context.object_vertex_buffer.handle, (VkDeviceSize*)offsets);
        
        // Bind index buffer at offset.
        vkCmdBindIndexBuffer(commandBuffer->handle, context.object_index_buffer.handle, 0, VK_INDEX_TYPE_UINT32);
        
        // Issue the draw.
        vkCmdDrawIndexed(commandBuffer->handle, 6, 1, 0, 0, 0);
    // TODO: end temporary test code
}

VKAPI_ATTR VkBool32 VKAPI_CALL vk_debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
    VkDebugUtilsMessageTypeFlagsEXT message_types,
    const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
    void* user_data) {

    switch(message_severity) {
        default:
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            HERROR(callback_data->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            HWARNING(callback_data->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
            HINFO(callback_data->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
            HTRACE(callback_data->pMessage);
            break;
    }
    return VK_FALSE;
}

i32 find_memory_index(u32 type_filter, u32 property_flags) {
    VkPhysicalDeviceMemoryProperties memory_properties;
    vkGetPhysicalDeviceMemoryProperties(context.device.physical_device, &memory_properties);

    for (u32 i = 0; i < memory_properties.memoryTypeCount; i++) {
        // Check each memory type to see if its bit is set to 1.
        if (type_filter & (1 << i) && (memory_properties.memoryTypes[i].propertyFlags & property_flags) == property_flags) {
            return i;
        }
    }

    HWARNING("Unable to find suitable memory type!");
    return -1;
}

void create_command_buffers(rendererBackend* backend) {
    if (!context.graphics_command_buffers) {
        context.graphics_command_buffers = darray_reserve(vulkanCommandBuffer, context.swapchain.imageCount);
        for (u32 i = 0; i < context.swapchain.imageCount; i++) {
            HzeroMemory(&context.graphics_command_buffers[i], sizeof(vulkanCommandBuffer));
        }
    }

    for (u32 i = 0; i < context.swapchain.imageCount; i++) {
        if (context.graphics_command_buffers[i].handle) {
            vulkanCommandBufferFree(
                &context,
                context.device.graphics_command_pool,
                &context.graphics_command_buffers[i]
            );
        }
        HzeroMemory(&context.graphics_command_buffers[i], sizeof(vulkanCommandBuffer));
        vulkanCommandBufferAllocate(
            &context,
            context.device.graphics_command_pool,
            true,
            &context.graphics_command_buffers[i]
        );
    }

    HDEBUG("Vulkan command buffers created.");
}

void regenerate_framebuffers(rendererBackend* backend, vulkanSwapchain* swapchain, vulkanRenderPass* renderpass) {
    for (u32 i = 0; i < swapchain->imageCount; i++) {
        // TODO: make this dinamic based on the currntly configured attachments
        u32 attachmentCount = 2;
        VkImageView attachments[] = {
            swapchain->views[i],
            swapchain->depth_attachment.view
        };

        vulkanFramebufferCreate(
            &context,
            renderpass,
            context.framebuffer_width,
            context.framebuffer_height,
            attachmentCount,
            attachments,
            &context.swapchain.framebuffers[i]
        );
    }
}

b8 recreateSwapchain(rendererBackend* backend) {
    // If already being recreated, don't do anything.
    if (context.recreating_swapchain) {
        HDEBUG("recreateSwapchain called when already being recreated. Booting.");
        return false;
    }
    
    // Detect if the window is too small to be drawn
    if (context.framebuffer_width == 0 || context.framebuffer_height == 0) {
        HDEBUG("recreateSwapchain failed called when window is < 1 in a dimension. Booting.");
        return false;
    }

    // Mark as recreating if the dimensions are valid.
    context.recreating_swapchain = true;

    // Wait for any operations to complete.
    vkDeviceWaitIdle(context.device.logical_device);

    for (u32 i = 0; i < context.swapchain.imageCount; i++) {
        context.imagesInFlight[i] = NULL;
    }

    // Requery support
    vulkanDeviceQuerySwapChainSupport(
        context.device.physical_device,
        context.surface,
        &context.device.swapchain_support
    );
    vulkanDeviceDetectDepthFormat(&context.device);

    recreateVulkanSwapchain(
        &context,
        cached_framebuffer_width,
        cached_framebuffer_height,
        &context.swapchain
    );

    // Sync the framebuffer size with the cached sizes.
    context.framebuffer_width = cached_framebuffer_width;
    context.framebuffer_height = cached_framebuffer_height;
    context.main_renderpass.w = context.framebuffer_width;
    context.main_renderpass.h = context.framebuffer_height;
    cached_framebuffer_width = 0;
    cached_framebuffer_height = 0;

    // Update framebuffer size generation.
    context.framebuffer_last_size_generation = context.framebuffer_size_generation;

    // Clean up the swapchain
    for (u32 i = 0; i < context.swapchain.imageCount; i++) {
        vulkanCommandBufferFree(
            &context, 
            context.device.graphics_command_pool, 
            &context.graphics_command_buffers[i]
        );
    }

    // Framebuffers.
    for (u32 i = 0; i < context.swapchain.imageCount; i++) {
        vulkanFramebufferDestroy(
            &context, 
            &context.swapchain.framebuffers[i]
        );
    }

    context.main_renderpass.x = 0;
    context.main_renderpass.y = 0;
    context.main_renderpass.w = context.framebuffer_width;
    context.main_renderpass.h = context.framebuffer_height;

    regenerate_framebuffers(backend, &context.swapchain, &context.main_renderpass);

    create_command_buffers(backend);

    // Clear the recreating flag.
    context.recreating_swapchain = false;

    HTRACE("swapchain recreated.");

    return false;
}

b8 create_buffers(vulkanContext* context) {
    VkMemoryPropertyFlagBits memory_property_flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

    const u64 vertex_buffer_size =  sizeof(vertex_3d) * 1024 * 1024;
    if (!vulkanBuffer_create(
            context, 
            vertex_buffer_size,
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            memory_property_flags,
            true,
            &context->object_vertex_buffer)) {
        HERROR("Error creating vertex buffer.");
        return false;
    }
    context->geometry_vertex_offset = 0;

    const u64 index_buffer_size = sizeof(u32) * 1024 * 1024;
    if (!vulkanBuffer_create(
            context,
            index_buffer_size,
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            memory_property_flags,
            true,
            &context->object_index_buffer)) {
        HERROR("Error creating index buffer.");
        return false;            
    }
    context->geometry_index_offset = 0;

    return true;
}