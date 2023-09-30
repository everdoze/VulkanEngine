#pragma once

#include <vulkan/vulkan.h>
#include "defines.hpp"

namespace Engine {

    typedef enum class VulkanCommandBufferState {
        READY,
        RECORDING,
        IN_RENDER_PASS,
        RECORDING_ENDED,
        SUBMITTED,
        NOT_ALLOCATED
    } VulkanCommandBufferState;

    class VulkanCommandBuffer {
        public:
            VkCommandBuffer handle;
            VulkanCommandBufferState state;

            VulkanCommandBuffer(
                VkCommandPool pool,
                b8 is_primary
            );

            ~VulkanCommandBuffer();

            void Begin(b8 is_single_use,
                       b8 is_renderpass_continue,
                       b8 is_simultaneous_use);

            void End();

            void UpdateSubmitted();

            void Reset();

            void BeginSingleUse();

            void EndSingleUse(VkQueue queue);

            void InRenderPass();

            void Recording();

        private:
            VkCommandPool pool;
    };  

};