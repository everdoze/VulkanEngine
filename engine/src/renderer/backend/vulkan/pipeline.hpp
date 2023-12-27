#pragma once

#include <vulkan/vulkan.h>
#include "defines.hpp"

namespace Engine {

    class VulkanPipeline {
        public:
            VkPipeline handle;
            VkPipelineLayout pipeline_layout;

            b8 ready;

            VulkanPipeline(
                class VulkanRenderpass* renderpass,
                u32 attribute_count,
                VkVertexInputAttributeDescription* attributes,
                u32 descriptor_set_layout_count,
                VkDescriptorSetLayout* descriptors_set_layouts,
                u32 stage_count,
                VkPipelineShaderStageCreateInfo* stages,
                u32 push_constant_range_count,
                MemoryRange* push_constant_ranges,
                u32 stride,
                VkViewport viewport, 
                VkRect2D scissor,
                b8 is_wireframe,
                b8 depth_test_enabled
            );

            ~VulkanPipeline();

            void Bind(class VulkanCommandBuffer* command_buffer, VkPipelineBindPoint bind_point);

        private:
            class VulkanRenderpass* renderpass;
    };

};