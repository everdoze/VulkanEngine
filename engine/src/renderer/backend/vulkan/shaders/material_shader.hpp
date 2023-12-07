#pragma once

#include <vulkan/vulkan.h>
#include "../pipeline.hpp"
#include "../buffer.hpp"
#include "../vulkan_texture.hpp"
#include "renderer/renderer_types.inl"
#include "resources/material/material.hpp"

#define VULKAN_MATERIAL_SHADER_STAGE_COUNT 2
#define VULKAN_MATERIAL_SHADER_DESCRIPTOR_COUNT 2
#define VULKAN_MATERIAL_SHADER_SAMPLER_COUNT 1
#define VULKAN_MATERIAL_MAX_OBJECT_COUNT 1024

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

    typedef struct VulkanMaterialShaderInstanceState {
        VkDescriptorSet descriptor_sets[3]; 
        VulkanDescriptorState descriptor_states[VULKAN_MATERIAL_SHADER_DESCRIPTOR_COUNT];
    } VulkanMaterialShaderInstanceState;

    class VulkanShader {
        public:
            VulkanShaderStage stages[VULKAN_MATERIAL_SHADER_STAGE_COUNT];
            VulkanPipeline* pipeline;

            VkDescriptorPool global_descriptor_pool;
            VkDescriptorSetLayout global_descriptor_set_layout;
            VkDescriptorSet global_descriptor_sets[3]; // 3 for tripple-buffering (one descriptor per frame)
            GlobalUniformObject global_ubo;
            VulkanBuffer* global_uniform_buffer;

            VkDescriptorSetLayout material_descriptor_set_layout;
            VkDescriptorPool material_descriptor_pool;
            VulkanBuffer* material_uniform_buffer;
            u32 material_uniform_buffer_index;

            b8 ready;

            std::vector<VulkanMaterialShaderInstanceState> instance_states;

            VulkanShader(std::string name);

            ~VulkanShader();

            void Use();

            void UpdateGlobalState();
            void UpdateObject(GeometryRenderData data);

            b8 AcquireResources(Material* material);
            void ReleaseResources(u32 material_id);

            b8 CreateShaderStage(
                std::string name,
                std::string type,
                VkShaderStageFlagBits shader_stage_flag,
                VulkanShaderStage* out_shader_stage
            );

            f32 accumulator = 0.0f;
            std::string name;
    };

};