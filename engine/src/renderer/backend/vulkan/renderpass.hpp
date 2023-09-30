#pragma once 

#include <vulkan/vulkan.h>
#include "defines.hpp"
#include "command_buffer.hpp"

namespace Engine {

    typedef enum class VulkanRenderPassState {
        READY,
        RECORDING,
        IN_RENDER_PASS,
        RECORDING_RENDERED,
        SUBMITTED,
        NOT_ALLOCATED
    } VulkanRenderPassState;

    class VulkanRenderpass {
        public:
            VkRenderPass handle;
            f32 x, y, w, h;
            f32 r, g, b, a;

            f32 depth;
            u32 stencil;

            VulkanRenderPassState state;

            b8 ready;

            VulkanRenderpass(
                f32 x, f32 y, f32 w, f32 h,
                f32 r, f32 g, f32 b, f32 a,
                f32 depth, u32 stencil);

            ~VulkanRenderpass();   

            void Begin(Ref<VulkanCommandBuffer> command_buffer);

            void End(Ref<VulkanCommandBuffer> command_buffer);
    };

};