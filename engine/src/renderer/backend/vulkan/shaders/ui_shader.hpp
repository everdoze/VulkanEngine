#pragma once

#include "base_shader.hpp"

#define VULKAN_UI_SHADER_STAGE_COUNT 2
#define VULKAN_UI_SHADER_DESCRIPTOR_COUNT 2
#define VULKAN_UI_SHADER_SAMPLER_COUNT 1
#define VULKAN_UI_MAX_OBJECT_COUNT 1024

#define BUILTIN_UI_SHADER_NAME "Builtin.UIShader"

namespace Engine {

    struct VulkanUIShaderInstanceState {
        VkDescriptorSet descriptor_sets[3]; 
        VulkanDescriptorState descriptor_states[VULKAN_UI_SHADER_DESCRIPTOR_COUNT];
    };

    class VulkanUIShader: public VulkanShader {
        public:
            VulkanShaderStage stages[VULKAN_UI_SHADER_STAGE_COUNT];
            VulkanPipeline* pipeline;

            VkDescriptorPool global_descriptor_pool;
            VkDescriptorSetLayout global_descriptor_set_layout;
            VkDescriptorSet global_descriptor_sets[3]; // 3 for tripple-buffering (one descriptor per frame)
            VulkanUIGlobalUBO global_ubo;
            VulkanBuffer* global_uniform_buffer;

            VkDescriptorSetLayout ui_descriptor_set_layout;
            VkDescriptorPool ui_descriptor_pool;
            VulkanBuffer* ui_uniform_buffer;
            u32 ui_uniform_buffer_index;

            std::vector<VulkanUIShaderInstanceState> instance_states;

            VulkanUIShader(std::string name);

            ~VulkanUIShader();

            void Use();

            void UpdateGlobalState();
            void UseModel(glm::mat4 model);
            void UseMaterial(Material* material);

            b8 AcquireResources(Material* material);
            void ReleaseResources(Material* material);
    };

}