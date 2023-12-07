#include "renderpass.hpp"
#include "vulkan.hpp"
#include "platform/platform.hpp"
#include "vulkan_helpers.hpp"
#include "core/logger/logger.hpp"
#include "swapchain.hpp"

namespace Engine {

   VulkanRenderpass::VulkanRenderpass(
        f32 x, f32 y, f32 w, f32 h,
        f32 r, f32 g, f32 b, f32 a,
        f32 depth, u32 stencil) {
        
        VulkanRendererBackend* backend = static_cast<VulkanRendererBackend*>(RendererFrontend::GetBackend());

        this->ready = false;

        this->x = x;
        this->y = y;
        this->w = w;
        this->h = h;
        
        this->r = r;
        this->g = g;
        this->b = b;
        this->a = a;

        this->depth = depth;
        this->stencil = stencil;

        // Main subpass
        VkSubpassDescription subpass = {};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

        // Attachments TODO: make configurable.
        const u32 attachment_description_count = 2;
        VkAttachmentDescription attachment_descriptions[attachment_description_count];

        // Color attachment
        VkAttachmentDescription color_attachment = {};
        color_attachment.format = backend->GetVulkanSwapchain()->image_format.format;
        color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
        color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        color_attachment.flags = 0;

        attachment_descriptions[0] = color_attachment;

        // Color attachment reference
        VkAttachmentReference color_attachment_reference = {};
        color_attachment_reference.attachment = 0;
        color_attachment_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &color_attachment_reference;


        // Depth attachment, if there is one
        VkAttachmentDescription depth_attachment = {};
        depth_attachment.format = backend->GetVulkanDevice()->depth_format;
        depth_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
        depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        attachment_descriptions[1] = depth_attachment;

        // Depth attachment reference
        VkAttachmentReference depth_attachment_reference = {};
        depth_attachment_reference.attachment = 1;
        depth_attachment_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        // TODO: other attachments types

        // Depth stencil data
        subpass.pDepthStencilAttachment = &depth_attachment_reference;

        // Input from a shader
        subpass.inputAttachmentCount = 0;
        subpass.pInputAttachments = 0;

        // Attachments used for multisampling color attachments
        subpass.pResolveAttachments = 0;

        // Attachments not used in this subpass, but must be preserved for the next.
        subpass.preserveAttachmentCount = 0;
        subpass.pPreserveAttachments = 0;

        // Render pass dependencies. TODO: make this configurable.
        VkSubpassDependency dependency;
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependency.dependencyFlags = 0;

        // Render pass
        VkRenderPassCreateInfo render_pass_create_info = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
        render_pass_create_info.attachmentCount = attachment_description_count;
        render_pass_create_info.pAttachments = attachment_descriptions;
        render_pass_create_info.subpassCount = 1;
        render_pass_create_info.pSubpasses = &subpass;
        render_pass_create_info.dependencyCount = 1;
        render_pass_create_info.pDependencies = &dependency;

        VK_CHECK(vkCreateRenderPass(
            backend->GetVulkanDevice()->logical_device,
            &render_pass_create_info,
            backend->GetVulkanAllocator(),
            &this->handle));

        DEBUG("Renderpass created successfully.");
        this->ready = true;
    };

    VulkanRenderpass::~VulkanRenderpass() {
        if (this->handle) {
            VulkanRendererBackend* backend = static_cast<VulkanRendererBackend*>(RendererFrontend::GetBackend());
            vkDestroyRenderPass(
                backend->GetVulkanDevice()->logical_device,
                this->handle,
                backend->GetVulkanAllocator());

            this->handle = nullptr;  
        }
    };

    void VulkanRenderpass::Begin(VulkanCommandBuffer* command_buffer) {
        VulkanRendererBackend* backend = static_cast<VulkanRendererBackend*>(RendererFrontend::GetBackend());
        VkRenderPassBeginInfo begin_info = {VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
        VulkanSwapchain* swapchain = backend->GetVulkanSwapchain();

        begin_info.renderPass = this->handle;
        begin_info.framebuffer = swapchain->framebuffers[backend->GetImageIndex()]->handle;
        begin_info.renderArea.offset.x = this->x;
        begin_info.renderArea.offset.y = this->y;
        begin_info.renderArea.extent.width = this->w;
        begin_info.renderArea.extent.height = this->h;

        const u32 clear_values_count = 2;
        VkClearValue clear_values[clear_values_count];
        Platform::ZMemory(clear_values, sizeof(VkClearValue) * clear_values_count);
        clear_values[0].color.float32[0] = this->r;
        clear_values[0].color.float32[1] = this->g;
        clear_values[0].color.float32[2] = this->b;
        clear_values[0].color.float32[3] = this->a;
        clear_values[1].depthStencil.depth = this->depth;
        clear_values[1].depthStencil.stencil = this->stencil;

        begin_info.clearValueCount = 2;
        begin_info.pClearValues = clear_values;

        vkCmdBeginRenderPass(command_buffer->handle, &begin_info, VK_SUBPASS_CONTENTS_INLINE);
        command_buffer->InRenderPass();
    };

    void VulkanRenderpass::End(VulkanCommandBuffer* command_buffer) {
        vkCmdEndRenderPass(command_buffer->handle);
        command_buffer->Recording();
    };

};