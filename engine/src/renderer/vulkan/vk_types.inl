#pragma once

#include "defines.h"
#include "core/asserts.h"

#include "renderer/types.inl"

#include <vulkan/vulkan.h>

// Checks the given expresion's return value against VK_SUCCESS
#define VK_CHECK(expr) {             \
    HASSERT(expr == VK_SUCCESS);     \
}

typedef struct vulkanBuffer {
    u64 total_size;
    VkBuffer handle;
    VkBufferUsageFlagBits usage;
    b8 is_locked;
    VkDeviceMemory memory;
    i32 memory_index;
    u32 memory_property_flags;
} vulkanBuffer;

typedef struct vk_swapchain_support_info {
    VkSurfaceCapabilitiesKHR capabilities;
    u32 format_count;
    VkSurfaceFormatKHR* formats;
    u32 present_mode_count;
    VkPresentModeKHR* present_modes;
} vk_swapchain_support_info;

typedef struct vulkanDevice {
    VkPhysicalDevice physical_device;
    VkDevice logical_device;
    vk_swapchain_support_info swapchain_support;
    
    i32 graphics_queue_index;
    i32 present_queue_index;
    i32 transfer_queue_index;

    VkQueue graphics_queue;
    VkQueue present_queue;
    VkQueue transfer_queue;

    VkCommandPool graphics_command_pool;

    VkPhysicalDeviceProperties properties;
    VkPhysicalDeviceFeatures features;
    VkPhysicalDeviceMemoryProperties memory;

    VkFormat depth_format;
} vulkanDevice;

typedef struct vulkanImage {
    VkImage handle;
    VkDeviceMemory memory;
    VkImageView view;
    u32 width;
    u32 height;
} vulkanImage;

typedef enum vulkanRenderPassState {
    READY, 
    RECORDING,
    IN_RENDER_PASS,
    RECORDING_ENDED,
    SUBMITED,
    NOT_ALLOCATED
} vulkanRenderPassState;

typedef struct vulkanRenderPass {
    VkRenderPass handle;
    f32 x, y, w, h;
    f32 r, g, b, a;

    f32 depth;
    u32 stencil;

    vulkanRenderPassState state;
} vulkanRenderPass;

typedef struct vulkanFramebuffer {
    VkFramebuffer handle;
    u32 attachmentCount;
    VkImageView* attachments;
    vulkanRenderPass* renderpass;
} vulkanFramebuffer;

typedef struct vulkanSwapchain {
    VkSurfaceFormatKHR imageFormat;
    u8 max_frames_in_flight;
    VkSwapchainKHR handle;
    u32 imageCount;
    VkImage *images;
    VkImageView *views;

    vulkanImage depth_attachment;

    // framebuffers used for on-screen rendering
    vulkanFramebuffer* framebuffers;
} vulkanSwapchain;

typedef enum vulkanCommandBufferState {
    COMMAND_BUFFER_STATE_READY, 
    COMMAND_BUFFER_STATE_RECORDING,
    COMMAND_BUFFER_STATE_IN_RENDER_PASS,
    COMMAND_BUFFER_STATE_RECORDING_ENDED,
    COMMAND_BUFFER_STATE_SUBMITTED,
    COMMAND_BUFFER_STATE_NOT_ALLOCATED
} vulkanCommandBufferState;

typedef struct vulkanCommandBuffer {
    VkCommandBuffer handle;

    // Command buffer state
    vulkanCommandBufferState state;
} vulkanCommandBuffer;

typedef struct vulkanFence {
    VkFence handle;
    b8 isSignaled;
} vulkanFence;

typedef struct vkShaderStage {
    VkShaderModuleCreateInfo createInfo;
    VkShaderModule handle;
    VkPipelineShaderStageCreateInfo shaderStageCreateInfo;
} vkShaderStage;

typedef struct vulkanPipeline {
    VkPipeline handle;
    VkPipelineLayout pipelineLayout;
} vulkanPipeline;

#define OBJECT_SHADER_STAGE_COUNT 2
typedef struct vkObjectShader {
    // vertex, fragment
    vkShaderStage stages[OBJECT_SHADER_STAGE_COUNT];

    VkDescriptorPool globalDescriptorPool;
    VkDescriptorSetLayout globalDescriptorSetLayout;

    // One descriptor set per frame (max 3 for triple-buffering)
    VkDescriptorSet globalDescriptorSets[3];

    // Global uniform object
    globalUniformObject global_ubo;

    // Global uniform buffer
    vulkanBuffer globalUniformBuffer;

    vulkanPipeline pipeline;
} vkObjectShader;

typedef struct vulkanContext {
    // Framebuffer properties
    u32 framebuffer_width;
    u32 framebuffer_height;

    // Current generation of framebuffer size, If it dooes not match
    // framebuffer_size_last_generation, a new one should be created.
    u64 framebuffer_size_generation; 

    // The generation of the framebuffer when it was last created.
    // Set to framebuffer_size_generation when it is updated.
    u64 framebuffer_last_size_generation;

    VkInstance instance;
    VkAllocationCallbacks* allocator;
    VkSurfaceKHR surface;

    #if defined(_DEBUG)
        // Just debugging stuff
        VkDebugUtilsMessengerEXT debug_messenger;
    #endif
    
    vulkanDevice device;
    
    vulkanSwapchain swapchain;
    vulkanRenderPass main_renderpass;

    vulkanBuffer object_vertex_buffer;
    vulkanBuffer object_index_buffer;

    // darray
    vulkanCommandBuffer* graphics_command_buffers;

    // darray
    VkSemaphore* imageAvailableSemaphores;
    VkSemaphore* queueCompleteSemaphores;

    u32 inFlightFenceCount;
    vulkanFence* inFlightFences;

    // Holds pointers to fences wich exist and are owned elsewhere
    vulkanFence** imagesInFlight;

    u32 imageIndex;
    u32 curFrame;

    b8 recreating_swapchain;

    vkObjectShader objectShader;

    u64 geometry_vertex_offset;
    u64 geometry_index_offset;

    i32 (*find_memory_index)(u32 type_filter, u32 property_flags);
} vulkanContext;