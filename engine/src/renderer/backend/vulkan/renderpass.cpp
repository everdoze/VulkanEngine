#include "renderpass.hpp"
#include "vulkan.hpp"
#include "platform/platform.hpp"
#include "helpers.hpp"
#include "core/logger/logger.hpp"
#include "swapchain.hpp"

namespace Engine {

    b8 operator&(const VulkanRenderPassClearFlag& value, const VulkanRenderPassClearFlag& operable) {
        return (u8)value & (u8)operable;
    };

    VulkanRenderPassClearFlag operator|(const VulkanRenderPassClearFlag& value, const VulkanRenderPassClearFlag& operable) {
        return (VulkanRenderPassClearFlag)((u8)value | (u8)operable);
    };

   VulkanRenderpass::VulkanRenderpass(
        std::string name,
        glm::vec4 render_area,
        glm::vec4 clear_color,
        f32 depth, u32 stencil,
        VulkanRenderPassClearFlag clear_flags,
        b8 has_prev_pass, b8 has_next_pass) {
        
        VulkanRendererBackend* backend = VulkanRendererBackend::GetInstance();

        this->ready = false;
        this->name = name;

        this->render_area = render_area;
        this->clear_color = clear_color;
        this->has_next_pass = has_next_pass;
        this->has_prev_pass = has_prev_pass;
        this->clear_flags = clear_flags;
        
        this->depth = depth;
        this->stencil = stencil;

        // Main subpass
        VkSubpassDescription subpass = {};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

        std::vector<VkAttachmentDescription> attachment_descriptions;
        // VkAttachmentDescription attachment_descriptions[attachment_description_count];

        // Color attachment
        b8 do_clear_color = this->clear_flags & VulkanRenderPassClearFlag::CLEAR_COLOR_BUFFER;
        VkAttachmentDescription color_attachment = {};
        color_attachment.format = backend->GetVulkanSwapchain()->image_format.format;
        color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
        color_attachment.loadOp = do_clear_color ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
        color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

        // If coming from a previous pass, should already be VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL. Otherwise undefined.
        color_attachment.initialLayout = has_prev_pass ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_UNDEFINED;
        // If going to another pass, use VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL. Otherwise VK_IMAGE_LAYOUT_PRESENT_SRC_KHR.
        color_attachment.finalLayout = has_next_pass ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        color_attachment.flags = 0;

        // Color attachment reference
        VkAttachmentReference color_attachment_reference = {};
        color_attachment_reference.attachment = attachment_descriptions.size();
        color_attachment_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &color_attachment_reference;
        attachment_descriptions.push_back(color_attachment);
        
        // Depth attachment, if there is one
        if (this->clear_flags & VulkanRenderPassClearFlag::CLEAR_COLOR_DEPTH_BUFER) {
            VkAttachmentDescription depth_attachment = {};
            depth_attachment.format = backend->GetVulkanDevice()->depth_format;
            depth_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
            depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

            // Depth attachment reference
            VkAttachmentReference depth_attachment_reference = {};
            depth_attachment_reference.attachment = attachment_descriptions.size();
            depth_attachment_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

            // Depth stencil data
            subpass.pDepthStencilAttachment = &depth_attachment_reference;
            attachment_descriptions.push_back(depth_attachment);
        } 
        
        // TODO: other attachments types

        

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
        render_pass_create_info.attachmentCount = attachment_descriptions.size();
        render_pass_create_info.pAttachments = attachment_descriptions.data();
        render_pass_create_info.subpassCount = 1;
        render_pass_create_info.pSubpasses = &subpass;
        render_pass_create_info.dependencyCount = 1;
        render_pass_create_info.pDependencies = &dependency;

        VK_CHECK(vkCreateRenderPass(
            backend->GetVulkanDevice()->logical_device,
            &render_pass_create_info,
            backend->GetVulkanAllocator(),
            &this->handle));

        DEBUG("Renderpass '%s' created successfully.", name.c_str());
        this->ready = true;
    };

    VulkanRenderpass::~VulkanRenderpass() {
        if (this->handle) {
            VulkanRendererBackend* backend = VulkanRendererBackend::GetInstance();
            
            vkDestroyRenderPass(
                backend->GetVulkanDevice()->logical_device,
                this->handle,
                backend->GetVulkanAllocator());

            this->handle = nullptr;  
        }
    };

    void VulkanRenderpass::OnResize(glm::vec4 render_area) {
        DEBUG("Updating '%s' renderpass render area: (%f, %f, %f, %f)", 
            name.c_str(), this->render_area.x, this->render_area.y,
            this->render_area.z, this->render_area.w);
        this->render_area = render_area;
    };

    void VulkanRenderpass::Begin(VulkanCommandBuffer* command_buffer, VulkanFramebuffer* framebuffer) {
        VkRenderPassBeginInfo begin_info = {VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};

        begin_info.renderPass = this->handle;
        begin_info.framebuffer = framebuffer->handle;
        begin_info.renderArea.offset.x = this->render_area.x;
        begin_info.renderArea.offset.y = this->render_area.y;
        begin_info.renderArea.extent.width = this->render_area.z;
        begin_info.renderArea.extent.height = this->render_area.w;
        
        std::vector<VkClearValue> clear_values;
        clear_values.reserve(2);
        begin_info.clearValueCount = clear_values.size();
        begin_info.pClearValues = nullptr;

        if (this->clear_flags & VulkanRenderPassClearFlag::CLEAR_COLOR_BUFFER) {
            VkClearValue cb_clear;
            Platform::CMemory(cb_clear.color.float32, &clear_color, sizeof(clear_color));
            clear_values.push_back(cb_clear);
        }

        if (this->clear_flags & VulkanRenderPassClearFlag::CLEAR_COLOR_DEPTH_BUFER) {
            VkClearValue db_clear;
            Platform::CMemory(db_clear.color.float32, &clear_color, sizeof(clear_color));
            db_clear.depthStencil.depth = this->depth;
            db_clear.depthStencil.stencil = 0;

            if (this->clear_flags & VulkanRenderPassClearFlag::CLEAR_COLOR_STENCIL_BUFFER) {
                db_clear.depthStencil.stencil = this->stencil;
            }
            clear_values.push_back(db_clear);
        }

        begin_info.clearValueCount = clear_values.size();
        begin_info.pClearValues = clear_values.data();

        vkCmdBeginRenderPass(command_buffer->handle, &begin_info, VK_SUBPASS_CONTENTS_INLINE);
        command_buffer->InRenderPass();
    };

    void VulkanRenderpass::End(VulkanCommandBuffer* command_buffer) {
        vkCmdEndRenderPass(command_buffer->handle);
        command_buffer->Recording();
    };

};