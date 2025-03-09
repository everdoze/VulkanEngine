#include "render_target.hpp"
#include "texture.hpp"
#include "renderpass.hpp"

namespace Engine {

    VulkanRenderTarget::VulkanRenderTarget(RenderTargetCreateInfo& info) {
        name = info.name;
        attachments = info.attachments;
        manage_attachments = info.manage_attachments;
        VkImageView views[32];
        for (u32 i = 0; i < this->attachments.size(); ++i) {
            views[i] = static_cast<VulkanTexture*>(this->attachments[i])->GetImage()->view;
        }

        framebuffer = new VulkanFramebuffer(
            static_cast<VulkanRenderpass*>(info.renderpass), 
            info.width, info.height,
            attachments.size(), views
        );
    }

    VulkanRenderTarget::~VulkanRenderTarget() {
        if (this->framebuffer) {
            delete this->framebuffer;
        }

        if (manage_attachments) {
            for (u32 i = 0; i < attachments.size(); ++i) {
                delete attachments[i];
            }
        }

        attachments.clear();
    }   
}
