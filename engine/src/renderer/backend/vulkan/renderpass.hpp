#pragma once 

#include <vulkan/vulkan.h>
#include "defines.hpp"
#include "command_buffer.hpp"
#include "framebuffer.hpp"

namespace Engine {

    enum class VulkanRenderPassState {
        READY,
        RECORDING,
        IN_RENDER_PASS,
        RECORDING_RENDERED,
        SUBMITTED,
        NOT_ALLOCATED
    };

    enum class VulkanRenderPassClearFlag {
        CLEAR_NONE = 0x0,
        CLEAR_COLOR_BUFFER = 0x1,
        CLEAR_COLOR_DEPTH_BUFER = 0x2,
        CLEAR_COLOR_STENCIL_BUFFER = 0x4
    };

    b8 operator&(const VulkanRenderPassClearFlag& value, const VulkanRenderPassClearFlag& operable);

    VulkanRenderPassClearFlag operator|(const VulkanRenderPassClearFlag& value, const VulkanRenderPassClearFlag& operable);

    class VulkanRenderpass {
        public:
            VkRenderPass handle;
            glm::vec4 render_area;
            glm::vec4 clear_color;

            f32 depth;
            u32 stencil;

            b8 has_prev_pass;
            b8 has_next_pass;

            VulkanRenderPassClearFlag clear_flags;

            VulkanRenderPassState state;

            b8 ready;

            std::string name;

            VulkanRenderpass(
                std::string name,
                glm::vec4 render_area,
                glm::vec4 clear_color,
                f32 depth, u32 stencil,
                VulkanRenderPassClearFlag clear_flags,
                b8 has_prev_pass, b8 has_next_pass);

            ~VulkanRenderpass();   

            void Begin(VulkanCommandBuffer* command_buffer, VulkanFramebuffer* framebuffer);
            void End(VulkanCommandBuffer* command_buffer);

            void OnResize(glm::vec4 render_area);
    };

};