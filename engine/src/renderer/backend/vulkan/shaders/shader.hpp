#pragma once

#include "defines.hpp"
#include <vulkan/vulkan.h>
#include "../pipeline.hpp"
#include "../buffer.hpp"
#include "../texture.hpp"
#include "renderer/renderer_types.hpp"
#include "renderer/backend/vulkan/vulkan_types.inl"
#include "resources/material/material.hpp"

#define VULKAN_SHADER_MAX_OBJECT_COUNT 1024
/** @brief The maximum number of stages (such as vertex, fragment, compute, etc.) allowed. */
#define VULKAN_SHADER_MAX_STAGES 8
/** @brief The maximum number of textures allowed at the global level. */
#define VULKAN_SHADER_MAX_GLOBAL_TEXTURES 31
/** @brief The maximum number of textures allowed at the instance level. */
#define VULKAN_SHADER_MAX_INSTANCE_TEXTURES 31
/** @brief The maximum number of vertex input attributes allowed. */
#define VULKAN_SHADER_MAX_ATTRIBUTES 16
/**
 * @brief The maximum number of uniforms and samplers allowed at the
 * global, instance and local levels combined. It's probably more than
 * will ever be needed.
 */
#define VULKAN_SHADER_MAX_UNIFORMS 128

/** @brief The maximum number of bindings per descriptor set. */
#define VULKAN_SHADER_MAX_BINDINGS 2
/** @brief The maximum number of push constant ranges for a shader. */
#define VULKAN_SHADER_MAX_PUSH_CONST_RANGES 32


namespace Engine {

    struct VulkanShaderStage {
        VkShaderModuleCreateInfo create_info;
        VkShaderModule handle;
        VkPipelineShaderStageCreateInfo shader_stage_create_info;
    };

    struct VulkanShaderDescriptorState {
        // One per frame
        u8 generations[3];
        u32 ids[3];
    };

    struct VulkanShaderDescriptorSetState {
        VkDescriptorSet descriptor_sets[3];
        VulkanShaderDescriptorState descriptor_states[VULKAN_SHADER_MAX_BINDINGS];
    };

    struct VulkanShaderInsanceState {
        u32 id;
        u64 offset;
        FreelistNode* allocated_block;
        VulkanShaderDescriptorSetState descriptor_set_state;
        std::vector<TextureMap*> instance_texture_maps;
    };

    enum class VulkanShaderDescriptorBindingFlags {
        UBO = 0x01,
        SAMPLER = 0x02,
    };

    ENABLE_BITMASK_OPERATORS(VulkanShaderDescriptorBindingFlags)

    enum class VulkanShaderDescriptorBindingIndex {
        UBO = 0x00,
        SAMPLER = 0x01
    };

    ENABLE_BITMASK_OPERATORS(VulkanShaderDescriptorBindingIndex)

    struct VulkanShaderDescrptorSetConfig {
        VulkanShaderDescriptorBindingFlags flags;
        std::vector<VkDescriptorSetLayoutBinding> bindings;

        b8 operator!= (i32 i) { return (u32)flags != i; };
        b8 operator== (i32 i) { return (u32)flags == i; };
    };

    struct VulkanShaderConfig {
        VulkanRenderpass* renderpass;
        std::vector<VkDescriptorPoolSize> pool_sizes;
        u16 max_descriptor_set_count;
        VulkanShaderDescrptorSetConfig descriptor_sets[(u32)ShaderScope::LENGTH];
        std::vector<VkVertexInputAttributeDescription> attributes;
    };

    class VulkanShader : public Shader {
        public:
            static const VkFormat attributes_formats[(u32)ShaderAttributeType::LENGTH];

            VulkanShader(VulkanShaderConfig& vk_config, ShaderConfig& config);

            ~VulkanShader();

            void Use();
            void BindGlobals();
            void BindInstance(u32 instance_id);

            void ApplyGlobals();
            void ApplyInstance(b8 needs_update);

            u32 AcquireInstanceResources(std::vector<TextureMap*> texture_maps);
            void ReleaseInstanceResources(u32 instance_id);

            b8 SetUniform(ShaderUniformConfig* uniform, const void* value);

            b8 CreateShaderStage(
                ShaderStageConfig& config,
                VulkanShaderStage* out_shader_stage
            );

            VulkanRenderpass* renderpass;
            VulkanShaderDescrptorSetConfig descriptor_sets[(u32)ShaderScope::LENGTH];
            std::vector<VkDescriptorPoolSize> pool_sizes;
            std::vector<VkVertexInputAttributeDescription> attributes;

            VkDescriptorPool descriptor_pool;
            VkDescriptorSetLayout descriptor_set_layouts[(u32)ShaderScope::LENGTH];
            VkDescriptorSet global_descriptor_sets[3];

            std::vector<VulkanShaderStage> stages;

            FreelistNode* global_ubo_block;

            VulkanBuffer* uniform_buffer;
            VulkanPipeline* pipeline;

            u16 max_descriptor_set_count;

        private:
            void* mapped_uniform_buffer_block;

            std::vector<VulkanShaderInsanceState> instance_states;

            u64 bound_ubo_offset;
            u64 bound_instance_id;

            VkShaderStageFlagBits GetVkStageType (ShaderStageConfig& stage);
            u32 GetInstanceId();

            u32 descriptor_set_count = 0;
    };

}