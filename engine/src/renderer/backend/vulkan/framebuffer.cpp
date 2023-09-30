#include "framebuffer.hpp"

#include "vulkan.hpp"
#include "renderpass.hpp"
#include "platform/platform.hpp"

#include "vulkan_helpers.hpp"
#include "core/logger/logger.hpp"

namespace Engine {

    VulkanFramebuffer::VulkanFramebuffer(
        Ref<VulkanRenderpass> renderpass,
        u32 width,
        u32 height,
        u32 attachment_count,
        VkImageView* attachments) {
        
        Ref<VulkanRendererBackend> backend = Cast<VulkanRendererBackend>(RendererFrontend::GetBackend());

        this->renderpass = renderpass;

        this->attachments = (VkImageView*)Platform::AMemory(sizeof(VkImageView) * attachment_count);
        for (u32 i = 0; i < attachment_count; ++i) {
            this->attachments[i] = attachments[i];
        }
        this->renderpass = renderpass;
        this->attachment_count = attachment_count;

        VkFramebufferCreateInfo fb_create_info = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
        fb_create_info.renderPass = renderpass->handle;
        fb_create_info.attachmentCount = attachment_count;
        fb_create_info.pAttachments = this->attachments;
        fb_create_info.width = width;
        fb_create_info.height = height;
        fb_create_info.layers = 1;
        
        VK_CHECK(vkCreateFramebuffer(
            backend->GetVulkanDevice()->logical_device,
            &fb_create_info,
            backend->GetVulkanAllocator(),
            &this->handle));
            
    };

    VulkanFramebuffer::~VulkanFramebuffer() {
        Ref<VulkanRendererBackend> backend = Cast<VulkanRendererBackend>(RendererFrontend::GetBackend());
        vkDestroyFramebuffer(
            backend->GetVulkanDevice()->logical_device,
            this->handle,
            backend->GetVulkanAllocator());

        if (this->attachments) {
            Platform::FMemory(this->attachments);
            this->attachments = nullptr;
        }
        
        this->attachment_count = 0;
        this->handle = nullptr;
        this->renderpass = nullptr;
    };

};