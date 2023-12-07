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
                VkViewport viewport, 
                VkRect2D scissor,
                b8 is_wireframe
            );

            ~VulkanPipeline();

            void Bind(class VulkanCommandBuffer* command_buffer, VkPipelineBindPoint bind_point);

        private:
            class VulkanRenderpass* renderpass;
    };

};