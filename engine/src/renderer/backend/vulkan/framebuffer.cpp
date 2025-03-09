#include "framebuffer.hpp"

#include "vulkan.hpp"
#include "renderpass.hpp"
#include "platform/platform.hpp"

#include "helpers.hpp"
#include "core/logger/logger.hpp"

namespace Engine {

    VulkanFramebuffer::VulkanFramebuffer(
        VulkanRenderpass* renderpass,
        u32 width,
        u32 height,
        u32 attachment_count,
        VkImageView* attachments) {

        this->renderpass = renderpass;

        this->attachments = (VkImageView*)Platform::AllocMemory(sizeof(VkImageView) * attachment_count);
        for (u32 i = 0; i < attachment_count; ++i) {
            this->attachments[i] = attachments[i];
        }
        this->attachment_count = attachment_count;

        CreateVulkanFramebuffer(width, height);
    };

    void VulkanFramebuffer::CreateVulkanFramebuffer(u32 width, u32 height) {
        VulkanRendererBackend* backend = VulkanRendererBackend::GetInstance();

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
    }

    void VulkanFramebuffer::DestroyVulkanFramebuffer() {
        VulkanRendererBackend* backend = VulkanRendererBackend::GetInstance();

        vkDestroyFramebuffer(
            backend->GetVulkanDevice()->logical_device,
            this->handle,
            backend->GetVulkanAllocator());
    };

    void VulkanFramebuffer::Regenerate(u32 width, u32 height) {
        DestroyVulkanFramebuffer();
        CreateVulkanFramebuffer(width, height);
    }

    VulkanFramebuffer::~VulkanFramebuffer() {
        VulkanRendererBackend* backend = VulkanRendererBackend::GetInstance();

        DestroyVulkanFramebuffer();
        
        if (this->attachments) {
            Platform::FrMemory(this->attachments);
            this->attachments = nullptr;
        }
        
        this->attachment_count = 0;
        this->handle = nullptr;
        this->renderpass = nullptr;
    };

};