#pragma once

#include "defines.hpp"
#include <vulkan/vulkan.h>
#include "../pipeline.hpp"
#include "../buffer.hpp"
#include "../texture.hpp"
#include "renderer/renderer_types.inl"
#include "renderer/backend/vulkan/vulkan_types.inl"
#include "resources/material/material.hpp"

namespace Engine {

    typedef struct VulkanShaderStage {
        VkShaderModuleCreateInfo create_info;
        VkShaderModule handle;
        VkPipelineShaderStageCreateInfo shader_stage_create_info;
    } VulkanShaderStage;

    typedef struct VulkanDescriptorState {
        // One per frame
        u32 generations[3];
        u32 ids[3];
    } VulkanDescriptorState;

    class VulkanShader {
        public:
            VulkanShader(std::string name);

            ~VulkanShader();

            virtual void Use() = 0;

            virtual void UpdateGlobalState() = 0;
            virtual void UseModel(glm::mat4 model) = 0;
            virtual void UseMaterial(Material* material) = 0;

            virtual b8 AcquireResources(Material* material) = 0;
            virtual void ReleaseResources(Material* material) = 0;

            b8 CreateShaderStage(
                std::string name,
                std::string type,
                VkShaderStageFlagBits shader_stage_flag,
                VulkanShaderStage* out_shader_stage
            );

            std::string name;
            b8 ready;
    };

}