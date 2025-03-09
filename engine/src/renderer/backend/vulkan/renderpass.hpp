#pragma once 

#include <vulkan/vulkan.h>
#include "defines.hpp"
#include "command_buffer.hpp"
#include "framebuffer.hpp"
#include "renderer/renderpass.hpp"

namespace Engine {

    enum class VulkanRenderPassState {
        READY,
        RECORDING,
        IN_RENDER_PASS,
        RECORDING_RENDERED,
        SUBMITTED,
        NOT_ALLOCATED
    };

    class VulkanRenderpass : public Renderpass {
        public:
            VkRenderPass handle;

            f32 depth;
            u32 stencil;

            b8 has_prev_pass;
            b8 has_next_pass;

            RenderpassClearFlag clear_flags;

            VulkanRenderPassState state;

            b8 ready;

            VulkanRenderpass(
                std::string name,
                glm::vec4 render_area,
                glm::vec4 clear_color,
                f32 depth, u32 stencil,
                RenderpassClearFlag clear_flags,
                b8 has_prev_pass, b8 has_next_pass);

            ~VulkanRenderpass();   

            b8 Begin() override;
            b8 End() override;

            void OnResize(glm::vec4 render_area) override;
    };

};