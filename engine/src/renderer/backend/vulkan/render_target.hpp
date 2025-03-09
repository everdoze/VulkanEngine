#pragma once
#include "defines.hpp"
#include "framebuffer.hpp"
#include "renderer/render_target.hpp"
#include "renderer/renderpass.hpp"

namespace Engine {

    class VulkanRenderTarget : public RenderTarget {
        public:
            VulkanRenderTarget(RenderTargetCreateInfo& info);
            ~VulkanRenderTarget();

            VulkanFramebuffer* GetFramebuffer() { return framebuffer; }
        protected:
            VulkanFramebuffer* framebuffer;
            b8 manage_attachments;
    };

}