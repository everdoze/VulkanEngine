#pragma once

#include "base_shader.hpp"

#define VULKAN_MATERIAL_SHADER_STAGE_COUNT 2
#define VULKAN_MATERIAL_SHADER_DESCRIPTOR_COUNT 2
#define VULKAN_MATERIAL_SHADER_SAMPLER_COUNT 1
#define VULKAN_MATERIAL_MAX_OBJECT_COUNT 1024

#define BUILTIN_MATERIAL_SHADER_NAME "Builtin.MaterialShader"

namespace Engine {

    struct VulkanMaterialShaderInstanceState {
        VkDescriptorSet descriptor_sets[3]; 
        VulkanDescriptorState descriptor_states[VULKAN_MATERIAL_SHADER_DESCRIPTOR_COUNT];
    };

    class VulkanMaterialShader: public VulkanShader {
        public:
            VulkanShaderStage stages[VULKAN_MATERIAL_SHADER_STAGE_COUNT];
            VulkanPipeline* pipeline;

            VkDescriptorPool global_descriptor_pool;
            VkDescriptorSetLayout global_descriptor_set_layout;
            VkDescriptorSet global_descriptor_sets[3]; // 3 for tripple-buffering (one descriptor per frame)
            VulkanMaterialGlobalUBO global_ubo;
            VulkanBuffer* global_uniform_buffer;

            VkDescriptorSetLayout material_descriptor_set_layout;
            VkDescriptorPool material_descriptor_pool;
            VulkanBuffer* material_uniform_buffer;
            u32 material_uniform_buffer_index;

            std::vector<VulkanMaterialShaderInstanceState> instance_states;

            VulkanMaterialShader(std::string name);

            ~VulkanMaterialShader();

            void Use();

            void UpdateGlobalState();
            void UseModel(glm::mat4 model);
            void UseMaterial(Material* material);

            b8 AcquireResources(Material* material);
            void ReleaseResources(Material* material);
    };

};