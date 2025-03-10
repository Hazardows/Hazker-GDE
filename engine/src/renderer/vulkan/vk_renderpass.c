#include "renderer/vulkan/vk_renderpass.h"

#include "memory/hmemory.h"

void createVulkanRenderpass(
    vulkanContext *context,
    vulkanRenderPass *renderpass,
    f32 x, f32 y, f32 w, f32 h,
    f32 r, f32 g, f32 b, f32 a,
    f32 depth, 
    u32 stencil) {

    renderpass->x = x;  
    renderpass->y = y;
    renderpass->w = w;
    renderpass->h = h;

    renderpass->r = r;
    renderpass->g = g;
    renderpass->b = b;
    renderpass->a = a;

    renderpass->depth = depth;
    renderpass->stencil = stencil;

    // Main subpass
    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

    // Attachments //TODO: Make this configurable
    #define attachmentDescriptionCount 2
    VkAttachmentDescription attachmentDescriptions[attachmentDescriptionCount];

    // Color attachment
    VkAttachmentDescription colorAttachment;
    colorAttachment.format = context->swapchain.imageFormat.format; // TODO: Configurable
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;      // Do not expect any particular layout before render pass starts.
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;  // Transitioned to after the render pass.
    colorAttachment.flags = 0;

    attachmentDescriptions[0] = colorAttachment;

    VkAttachmentReference colorAttachmentReference;
    colorAttachmentReference.attachment = 0;    // Attachment description array index
    colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentReference;

    // Depth attachment (if there is one)
    VkAttachmentDescription depthAttachment = {};
    depthAttachment.format = context->device.depth_format;
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    attachmentDescriptions[1] = depthAttachment;

    // Depth attachment reference
    VkAttachmentReference depthAttachmentReference;
    depthAttachmentReference.attachment = 1;
    depthAttachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    // TODO: other attachment types (input, resolve, preserve)

    // Depth stencil data.
    subpass.pDepthStencilAttachment = &depthAttachmentReference;

    // Input from a shader
    subpass.inputAttachmentCount = 0;
    subpass.pInputAttachments = NULL;

    // Attachments used for multisampling colour attachments
    subpass.pResolveAttachments = NULL;

    // Attachments not used in this subpass, but must be preserved for the next.
    subpass.preserveAttachmentCount = 0;
    subpass.pPreserveAttachments = NULL;

    // Render pass dependencies. // TODO: Make this configurable
    VkSubpassDependency dependency;
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependency.dependencyFlags = 0;

    // Render pass create info(RPCI)
    VkRenderPassCreateInfo RPCI = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
    RPCI.attachmentCount = attachmentDescriptionCount;
    RPCI.pAttachments = attachmentDescriptions;
    RPCI.subpassCount = 1;
    RPCI.pSubpasses = &subpass;
    RPCI.dependencyCount = 1;
    RPCI.pDependencies = &dependency;
    RPCI.pNext = NULL;
    RPCI.flags = 0;

    VK_CHECK(vkCreateRenderPass(
        context->device.logical_device,
        &RPCI,
        context->allocator,
        &renderpass->handle
    ));
}

void destroyVulkanRenderpass(
    vulkanContext *context, 
    vulkanRenderPass *renderpass) {
    
    if (renderpass && renderpass->handle) {
        vkDestroyRenderPass(context->device.logical_device, renderpass->handle, context->allocator);
        renderpass->handle = NULL;
    }
}

void vulkanRenderPassBegin(
    vulkanCommandBuffer *command_buffer,
    vulkanRenderPass *renderpass,
    VkFramebuffer framebuffer) {

    VkRenderPassBeginInfo begin_info = {VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
    begin_info.renderPass = renderpass->handle;
    begin_info.framebuffer = framebuffer;
    begin_info.renderArea.offset.x = renderpass->x;
    begin_info.renderArea.offset.y = renderpass->y;
    begin_info.renderArea.extent.width = renderpass->w;
    begin_info.renderArea.extent.height = renderpass->h;
    begin_info.pNext = NULL;

    VkClearValue clear_values[2];
    HzeroMemory(clear_values, sizeof(VkClearValue) * 2);
    clear_values[0].color.float32[0] = renderpass->r;
    clear_values[0].color.float32[1] = renderpass->g;
    clear_values[0].color.float32[2] = renderpass->b;
    clear_values[0].color.float32[3] = renderpass->a;
    clear_values[1].depthStencil.depth = renderpass->depth;
    clear_values[1].depthStencil.stencil = renderpass->stencil;

    begin_info.clearValueCount = 2;
    begin_info.pClearValues = clear_values;

    vkCmdBeginRenderPass(command_buffer->handle, &begin_info, VK_SUBPASS_CONTENTS_INLINE);
    command_buffer->state = COMMAND_BUFFER_STATE_IN_RENDER_PASS;
}

void vulkanRenderPassEnd(
    vulkanCommandBuffer *command_buffer,
    vulkanRenderPass *renderpass) {

    vkCmdEndRenderPass(command_buffer->handle);
    command_buffer->state = COMMAND_BUFFER_STATE_RECORDING;
}