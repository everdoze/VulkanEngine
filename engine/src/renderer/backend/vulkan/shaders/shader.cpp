#include "shader.hpp"

#include "core/utils/string.hpp"
#include "platform/platform.hpp"
#include "core/logger/logger.hpp"
#include "../helpers.hpp"
#include "../vulkan.hpp"
#include "systems/resource/resource_system.hpp"
#include "systems/texture/texture_system.hpp"
#include "../sampler.hpp"

namespace Engine {

    const VkFormat VulkanShader::attributes_formats[(u32)ShaderAttributeType::LENGTH] = {
        VK_FORMAT_UNDEFINED,
        VK_FORMAT_R32_SFLOAT,
        VK_FORMAT_R32G32_SFLOAT,
        VK_FORMAT_R32G32B32_SFLOAT,
        VK_FORMAT_R32G32B32A32_SFLOAT,
        VK_FORMAT_R8_SINT,
        VK_FORMAT_R16_SINT,
        VK_FORMAT_R32_SINT,
        VK_FORMAT_R8_UINT,
        VK_FORMAT_R16_UINT,
        VK_FORMAT_R32_UINT
    };

    VulkanShader::VulkanShader(VulkanShaderConfig& vk_config, ShaderConfig& config): Shader(config) {
        this->ready = false;

        VulkanRendererBackend* backend = VulkanRendererBackend::GetInstance();

        if (!vk_config.renderpass) {
            ERROR("VulkanShader::VulkanShader - renderpass is not provided in VulkanShaderConfig.");
            return;
        }

        this->renderpass = vk_config.renderpass;

        // Creating shader stages
        for (u32 i = 0; i < config.stages.size(); ++i) {
            VulkanShaderStage stage;
            if (!CreateShaderStage(config.stages[i], &stage)) {
                ERROR("VulkanShader::VulkanShader - failed to create shader stage '%s'", config.stages[i].name.c_str());
                return;
            }
            stages.push_back(stage);
        }


        this->pool_sizes = vk_config.pool_sizes;
        this->max_descriptor_set_count = vk_config.max_descriptor_set_count;

        descriptor_set_count = 1;
        if (config.use_instances) {
            descriptor_set_count++;
        }

        // Saving descriptor_sets
        for (u32 i = 0; i < descriptor_set_count; ++i) {
            this->descriptor_sets[i] = vk_config.descriptor_sets[i];
            this->descriptor_sets[i].bindings.reserve(VULKAN_SHADER_MAX_BINDINGS);
        }
        
        // Process attributes
        u32 offset = 0;
        for (u32 i = 0; i < config.attributes.size(); ++i) {
            VkVertexInputAttributeDescription attribute;
            attribute.location = i;
            attribute.binding = 0;
            attribute.offset = offset;
            attribute.format = attributes_formats[(u32)config.attributes[i].type];
            offset += config.attributes[i].size;
            attribute_stride += config.attributes[i].size;
            attributes.push_back(attribute);
        }
        
        // Descriptor pool.
        VkDescriptorPoolCreateInfo pool_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
        pool_info.poolSizeCount = 2;
        pool_info.pPoolSizes = pool_sizes.data();
        pool_info.maxSets = max_descriptor_set_count;
        pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

         // Create descriptor pool.
        VkResult result = vkCreateDescriptorPool(
            backend->GetVulkanDevice()->logical_device, 
            &pool_info, backend->GetVulkanAllocator(), 
            &descriptor_pool);
        if (!IsVulkanResultSuccess(result)) {
            ERROR("VulkanShader::VulkanShader - failed creating descriptor pool: '%s'", VulkanResultString(result, true));
            return;
        }

        // Create descriptor set layouts.
        Platform::ZrMemory(descriptor_set_layouts, (u32)ShaderScope::LENGTH * sizeof(VkDescriptorSetLayout));
        for (u32 i = 0; i < (u32)ShaderScope::LENGTH; ++i) {
            VkDescriptorSetLayoutCreateInfo layout_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
            layout_info.bindingCount = descriptor_sets[i].bindings.size();
            layout_info.pBindings = descriptor_sets[i].bindings.data();
            result = vkCreateDescriptorSetLayout(
                backend->GetVulkanDevice()->logical_device, 
                &layout_info, backend->GetVulkanAllocator(), 
                &descriptor_set_layouts[i]);
            if (!IsVulkanResultSuccess(result)) {
                ERROR("VulkanShader::VulkanShader - failed creating descriptor pool: '%s'", VulkanResultString(result, true));
                return;
            }
        }

        // TODO: This feels wrong to have these here, at least in this fashion. Should probably
        // Be configured to pull from someplace instead.

        // Pipeline creation
        VkViewport viewport;
        viewport.x = 0.0f;
        viewport.y = (f32)backend->GetFrameHeight();
        viewport.width = (f32)backend->GetFrameWidth();
        viewport.height = -(f32)backend->GetFrameHeight();
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        // Scissor
        VkRect2D scissor;
        scissor.offset.x = scissor.offset.y = 0;
        scissor.extent.width = backend->GetFrameWidth();
        scissor.extent.height = backend->GetFrameHeight();

        std::vector<VkPipelineShaderStageCreateInfo> pipeline_stage_create_info;
        pipeline_stage_create_info.resize(stages.size());
        for (u32 i = 0; i < stages.size(); ++i) {
            pipeline_stage_create_info[i] = stages[i].shader_stage_create_info;
        }

        pipeline = new VulkanPipeline(
            renderpass,
            attributes.size(),
            attributes.data(),
            descriptor_set_count,
            descriptor_set_layouts,
            stages.size(),
            pipeline_stage_create_info.data(),
            push_constant_count,
            push_constant_ranges,
            attribute_stride,
            viewport,
            scissor,
            false,
            true
        );
        
        if (!pipeline->ready) {
            ERROR("Failed to load graphics pipeline for object shader '%s'.", name.c_str());
            return;
        }

        // Get the UBO alignment
        VulkanDevice* device = backend->GetVulkanDevice();
        required_ubo_alignment = device->properties.limits.minUniformBufferOffsetAlignment;

        // Make sure the UBO is aligned according to device requirements.
        global_ubo_stride = GetAligned(global_ubo.size, required_ubo_alignment);
        ubo_stride = GetAligned(ubo.size, required_ubo_alignment);

        u32 device_local_bits = device->supports_device_local_host_visible ? VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT : 0;
        u64 buffer_size = global_ubo_stride + (ubo_stride * VULKAN_SHADER_MAX_OBJECT_COUNT);
        uniform_buffer = new VulkanBuffer(
            buffer_size,
            (VkBufferUsageFlagBits)(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT),
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | device_local_bits,
            true,
            true
        );

        global_ubo_block = uniform_buffer->Allocate(global_ubo_stride);

        if (!uniform_buffer->ready) {
            ERROR("Failed to create uniform_buffer for object shader '%s'.", name.c_str());
            return;
        }

        mapped_uniform_buffer_block = uniform_buffer->LockMemory(0, VK_WHOLE_SIZE, 0);

        VkDescriptorSetLayout global_layouts[3] = {
        descriptor_set_layouts[(u32)ShaderScope::GLOBAL],
        descriptor_set_layouts[(u32)ShaderScope::GLOBAL],
        descriptor_set_layouts[(u32)ShaderScope::GLOBAL]};

        VkDescriptorSetAllocateInfo alloc_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
        alloc_info.descriptorPool = descriptor_pool;
        alloc_info.descriptorSetCount = 3;
        alloc_info.pSetLayouts = global_layouts;
        VK_CHECK(vkAllocateDescriptorSets(device->logical_device, &alloc_info, global_descriptor_sets));

        this->ready = true;
    };

    VulkanShader::~VulkanShader() {
        VulkanRendererBackend* backend = VulkanRendererBackend::GetInstance();
        VulkanDevice* device = backend->GetVulkanDevice();

        for (u32 i = 0; i < instance_states.size(); ++i) {
            for (u32 j = 0; j < instance_states[i].instance_texture_maps.size(); ++j) {
                delete instance_states[i].instance_texture_maps[j]->sampler;
            }
        }

        // Descriptor set layouts.
        for (u32 i = 0; i < 3; ++i) {
            if (descriptor_set_layouts[i]) {
                vkDestroyDescriptorSetLayout(
                    device->logical_device, 
                    descriptor_set_layouts[i], 
                    backend->GetVulkanAllocator());
                descriptor_set_layouts[i] = 0;
            }
        }

        // Descriptor pool
        if (descriptor_pool) {
            vkDestroyDescriptorPool(
                device->logical_device, 
                descriptor_pool, 
                backend->GetVulkanAllocator());
        }

        // Uniform buffer.
        if (uniform_buffer) {
            global_ubo_block->FreeBlock();
            uniform_buffer->UnlockMemory();
            mapped_uniform_buffer_block = 0;

            delete uniform_buffer;
        }
        
        // Pipeline
        if (pipeline) {
            delete pipeline;
        }
        
        // Shader modules
        for (u32 i = 0; i < stages.size(); ++i) {
            vkDestroyShaderModule(
                device->logical_device, 
                stages[i].handle, 
                backend->GetVulkanAllocator());
        }

    };

    // VK_SHADER_STAGE_VERTEX_BIT = 0x00000001,
    // VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT = 0x00000002,
    // VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT = 0x00000004,
    // VK_SHADER_STAGE_GEOMETRY_BIT = 0x00000008,
    // VK_SHADER_STAGE_FRAGMENT_BIT = 0x00000010,
    // VK_SHADER_STAGE_COMPUTE_BIT = 0x00000020,
    // VK_SHADER_STAGE_ALL_GRAPHICS = 0x0000001F,
    // VK_SHADER_STAGE_ALL = 0x7FFFFFFF,
    // VK_SHADER_STAGE_RAYGEN_BIT_KHR = 0x00000100,
    // VK_SHADER_STAGE_ANY_HIT_BIT_KHR = 0x00000200,
    // VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR = 0x00000400,
    // VK_SHADER_STAGE_MISS_BIT_KHR = 0x00000800,
    // VK_SHADER_STAGE_INTERSECTION_BIT_KHR = 0x00001000,
    // VK_SHADER_STAGE_CALLABLE_BIT_KHR = 0x00002000,
    // VK_SHADER_STAGE_TASK_BIT_NV = 0x00000040,
    // VK_SHADER_STAGE_MESH_BIT_NV = 0x00000080,
    // VK_SHADER_STAGE_SUBPASS_SHADING_BIT_HUAWEI = 0x00004000,
    // VK_SHADER_STAGE_RAYGEN_BIT_NV = VK_SHADER_STAGE_RAYGEN_BIT_KHR,
    // VK_SHADER_STAGE_ANY_HIT_BIT_NV = VK_SHADER_STAGE_ANY_HIT_BIT_KHR,
    // VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR,
    // VK_SHADER_STAGE_MISS_BIT_NV = VK_SHADER_STAGE_MISS_BIT_KHR,
    // VK_SHADER_STAGE_INTERSECTION_BIT_NV = VK_SHADER_STAGE_INTERSECTION_BIT_KHR,
    // VK_SHADER_STAGE_CALLABLE_BIT_NV = VK_SHADER_STAGE_CALLABLE_BIT_KHR,
    // VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM = 0x7FFFFFFF

    VkShaderStageFlagBits VulkanShader::GetVkStageType (ShaderStageConfig& stage) {
        switch (stage.stage) {
            case ShaderStage::VERTEX:
                return VK_SHADER_STAGE_VERTEX_BIT;
            case ShaderStage::FRAGMENT:
                return VK_SHADER_STAGE_FRAGMENT_BIT;
            // case ShaderStage::GEOMETRY:
            //     return VK_SHADER_STAGE_GEOMETRY_BIT;
            // case ShaderStage::COMPUTE:
            //     return VK_SHADER_STAGE_COMPUTE_BIT;
            default:
                ERROR("VulkanShader::GetVkStageType - unsupported ShaderStage type: '%s'.", stage.name.c_str());
                return (VkShaderStageFlagBits)0;
        }
    };

    b8 VulkanShader::CreateShaderStage(
        ShaderStageConfig& config,
        VulkanShaderStage* out_shader_stage) {
        
        VulkanRendererBackend* backend = VulkanRendererBackend::GetInstance();

        Platform::ZrMemory(&out_shader_stage->create_info, sizeof(VkShaderModuleCreateInfo));

        std::string shader_name = config.file_path;
        BinaryResource* b_resource = static_cast<BinaryResource*>(
            ResourceSystem::GetInstance()->LoadResource(ResourceType::BINARY, shader_name)
        );

        if (!b_resource) {
            ERROR("Unable to read shader module: %s.", shader_name.c_str());
            return false;
        }

        std::vector<c8> bytes = b_resource->GetData();

        if (bytes.size() == 0) {
            ERROR("Unable to binary read shader module: %s.", shader_name.c_str());
            return false;
        }
        
        out_shader_stage->create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        out_shader_stage->create_info.codeSize = bytes.size();
        out_shader_stage->create_info.pCode = (u32*)bytes.data();

        VK_CHECK(vkCreateShaderModule(
            backend->GetVulkanDevice()->logical_device,
            &out_shader_stage->create_info,
            backend->GetVulkanAllocator(),
            &out_shader_stage->handle));

        delete b_resource;

        Platform::ZrMemory(&out_shader_stage->shader_stage_create_info, sizeof(VkPipelineShaderStageCreateInfo));
        out_shader_stage->shader_stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        out_shader_stage->shader_stage_create_info.stage = GetVkStageType(config);
        out_shader_stage->shader_stage_create_info.module = out_shader_stage->handle;
        out_shader_stage->shader_stage_create_info.pName = "main";

        return true;
    };

    void VulkanShader::Use() {
        VulkanRendererBackend* backend = VulkanRendererBackend::GetInstance();
        u32 image_index = backend->GetImageIndex();
        VulkanCommandBuffer* command_buffer = backend->GetGraphicsCommandBufers()[image_index];
        this->pipeline->Bind(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS);
    };


    u32 VulkanShader::GetInstanceId() {
        if (!instance_states.size()) {
            return 0;
        }
        u32 id = instance_states.size();
        for (u32 i = 0; i < id; ++i) {
            if (instance_states[i].id == INVALID_ID) {
                id = i;
                break;
            }
        }
        return id;
    };

    void VulkanShader::BindGlobals() {
        bound_ubo_offset = global_ubo.offset;
    };

    void VulkanShader::BindInstance(u32 instance_id) {
        bound_instance_id = instance_id;
        bound_ubo_offset = instance_states[instance_id].offset;
    };

    void VulkanShader::ApplyGlobals() {
        VulkanRendererBackend* backend = VulkanRendererBackend::GetInstance();
        VulkanDevice* device = backend->GetVulkanDevice();
        u32 image_index = backend->GetImageIndex();
        VulkanCommandBuffer* command_buffer = backend->GetGraphicsCommandBufers()[image_index];
        VkDescriptorSet* global_descriptor = &global_descriptor_sets[image_index];

        // Apply UBO first
        VkDescriptorBufferInfo buffer_info;
        buffer_info.buffer = uniform_buffer->handle;
        buffer_info.offset = global_ubo.offset;
        buffer_info.range = global_ubo_stride;

        // Update descriptor sets.
        VkWriteDescriptorSet ubo_write = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
        ubo_write.dstSet = global_descriptor_sets[image_index];
        ubo_write.dstBinding = 0;
        ubo_write.dstArrayElement = 0;
        ubo_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        ubo_write.descriptorCount = 1;
        ubo_write.pBufferInfo = &buffer_info;

        VkWriteDescriptorSet descriptor_writes[2] = {};
        descriptor_writes[0] = ubo_write;

        u32 global_set_binding_count = descriptor_sets[(u32)ShaderScope::GLOBAL].bindings.size();

        if (global_set_binding_count > 1) {
            // TODO: There are samplers to be written. Support this.
            global_set_binding_count = 1;
            ERROR("Global image samplers are not yet supported.");

            // VkWriteDescriptorSet sampler_write = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
            // descriptor_writes[1] = ...
        }

        vkUpdateDescriptorSets(device->logical_device, global_set_binding_count, descriptor_writes, 0, 0);

        // Bind the global descriptor set to be updated.
        vkCmdBindDescriptorSets(command_buffer->handle, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->pipeline_layout, 0, 1, global_descriptor, 0, 0);
    };

    void VulkanShader::ApplyInstance(b8 needs_update) {
        if (!use_instances) {
            ERROR("This shader does not use instances.");
            return;
        }

        VulkanRendererBackend* backend = VulkanRendererBackend::GetInstance();
        VulkanDevice* device = backend->GetVulkanDevice();
        u32 image_index = backend->GetImageIndex();
        VulkanCommandBuffer* command_buffer = backend->GetGraphicsCommandBufers()[image_index];

        VulkanShaderInsanceState* state = &instance_states[bound_instance_id];
        VkDescriptorSet object_descriptor_set = state->descriptor_set_state.descriptor_sets[image_index];

        if (needs_update) {
            VkWriteDescriptorSet descriptor_writes[2];  // TODO: Always a max of 2 descriptor sets, adding more needs to make it dynamic.
            Platform::ZrMemory(descriptor_writes, sizeof(VkWriteDescriptorSet) * 2);
            u32 descriptor_count = 0;
            u32 descriptor_index = 0;

            // Descriptor 0 - Uniform buffer
            // Only do this if the descriptor has not yet been updated.
            u8* instance_ubo_generation = &state->descriptor_set_state.descriptor_states[descriptor_index].generations[image_index];
            // TODO: determine if update is required.
            if (*instance_ubo_generation == INVALID_ID_U8 /*|| *global_ubo_generation != material->generation*/) {
                VkDescriptorBufferInfo buffer_info;
                buffer_info.buffer = uniform_buffer->handle;
                buffer_info.offset = state->offset;
                buffer_info.range = ubo_stride;

                VkWriteDescriptorSet ubo_descriptor = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
                ubo_descriptor.dstSet = object_descriptor_set;
                ubo_descriptor.dstBinding = descriptor_index;
                ubo_descriptor.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                ubo_descriptor.descriptorCount = 1;
                ubo_descriptor.pBufferInfo = &buffer_info;

                descriptor_writes[descriptor_count] = ubo_descriptor;
                descriptor_count++;

                // Update the frame generation. In this case it is only needed once since this is a buffer.
                *instance_ubo_generation = 1;  // material->generation; TODO: some generation from... somewhere
            }
            descriptor_index++;

            // Samplers will always be in the binding. If the binding count is less than 2, there are no samplers.
            if (descriptor_sets[(u32)ShaderScope::INSTANCE].flags & VulkanShaderDescriptorBindingFlags::SAMPLER) {
                // Iterate samplers.
                u32 total_sampler_count = descriptor_sets[(u32)ShaderScope::INSTANCE].bindings[(u32)VulkanShaderDescriptorBindingIndex::SAMPLER].descriptorCount;
                u32 update_sampler_count = 0;
                VkDescriptorImageInfo image_infos[VULKAN_SHADER_MAX_INSTANCE_TEXTURES];
                for (u32 i = 0; i < total_sampler_count; ++i) {
                    // TODO: only update in the list if actually needing an update.
                    TextureMap* map = instance_states[bound_instance_id].instance_texture_maps[i];
                    VulkanTexture* t = (VulkanTexture*)map->texture;
                    VulkanSampler* sampler = static_cast<VulkanSampler*>(map->sampler);
                    image_infos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                    image_infos[i].imageView = t->GetImage()->view;
                    image_infos[i].sampler = sampler->GetSampler();

                    // TODO: change up descriptor state to handle this properly.
                    // Sync frame generation if not using a default texture.
                    // if (t->generation != INVALID_ID) {
                    //     *descriptor_generation = t->generation;
                    //     *descriptor_id = t->id;
                    // }

                    update_sampler_count++;
                }

                VkWriteDescriptorSet sampler_descriptor = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
                sampler_descriptor.dstSet = object_descriptor_set;
                sampler_descriptor.dstBinding = descriptor_index;
                sampler_descriptor.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                sampler_descriptor.descriptorCount = update_sampler_count;
                sampler_descriptor.pImageInfo = image_infos;

                descriptor_writes[descriptor_count] = sampler_descriptor;
                descriptor_count++;
            }


            if (descriptor_count > 0) {
                vkUpdateDescriptorSets(device->logical_device, descriptor_count, descriptor_writes, 0, 0);
            }
        }
 
        // Bind the descriptor set to be updated, or in case the shader changed.
        vkCmdBindDescriptorSets(command_buffer->handle, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->pipeline_layout, 1, 1, &object_descriptor_set, 0, 0);
    };

    u32 VulkanShader::AcquireInstanceResources(std::vector<TextureMap*> texture_maps) {
        VulkanRendererBackend* backend = VulkanRendererBackend::GetInstance();
        VulkanDevice* device = backend->GetVulkanDevice();

        u32 instance_id = INVALID_ID;
        for (u32 i = 0; i < instance_states.size(); ++i) {
            if (instance_states[i].id == INVALID_ID) {
                instance_id = i;
            } 
        }
        VulkanShaderInsanceState* instance_state;
        if (instance_id == INVALID_ID) {
            VulkanShaderInsanceState inst_st = {};
            inst_st.id = instance_states.size();
            inst_st.offset = global_ubo_stride + ubo_stride * inst_st.id;
            instance_states.push_back(inst_st);
            instance_id = inst_st.id;
            instance_state = &instance_states[inst_st.id];
        } else {
            instance_state = &instance_states[instance_id];
        }

        u32 texture_count = descriptor_sets[(u32)ShaderScope::INSTANCE].bindings[(u32)VulkanShaderDescriptorBindingIndex::SAMPLER].descriptorCount;
        instance_state->instance_texture_maps.resize(texture_count);

        for (u32 i = 0; i < texture_count; ++i) {
            instance_state->instance_texture_maps[i] = texture_maps[i];
        }
        
        FreelistNode* node = uniform_buffer->Allocate(ubo_stride);
        if (!node) {
            ERROR("VulkanShader::AcquireInstanceResources - failed, can't allocate instance UBO in shader '%s", name.c_str());
            return INVALID_ID;
        }

        instance_state->allocated_block = node;

        VulkanShaderDescriptorSetState* set_state = &instance_state->descriptor_set_state;
        u32 binding_count = descriptor_sets[(u32)ShaderScope::INSTANCE].bindings.size();
        for (u32 i = 0; i < binding_count; ++i) {
            for (u32 j = 0; j < 3; ++j) {
                set_state->descriptor_states[i].generations[j] = INVALID_ID_U8;
                set_state->descriptor_states[i].ids[j] = INVALID_ID;
            }
        }

        VkDescriptorSetLayout layouts[3] = {
        descriptor_set_layouts[(u32)ShaderScope::INSTANCE],
        descriptor_set_layouts[(u32)ShaderScope::INSTANCE],
        descriptor_set_layouts[(u32)ShaderScope::INSTANCE]};

        VkDescriptorSetAllocateInfo alloc_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
        alloc_info.descriptorPool = descriptor_pool;
        alloc_info.descriptorSetCount = 3;
        alloc_info.pSetLayouts = layouts;
        VkResult result = vkAllocateDescriptorSets(
            device->logical_device,
            &alloc_info,
            instance_state->descriptor_set_state.descriptor_sets);
        if (result != VK_SUCCESS) {
            ERROR("Error allocating instance descriptor sets in shader: '%s'.", VulkanResultString(result, true));
            return false;
        }

        return instance_id;
    };

    void VulkanShader::ReleaseInstanceResources(u32 instance_id) {
        VulkanRendererBackend* backend = VulkanRendererBackend::GetInstance();
        VulkanDevice* device = backend->GetVulkanDevice();

        VulkanShaderInsanceState* state = &instance_states[instance_id];

        vkDeviceWaitIdle(device->logical_device);

        // Free 3 descriptor sets (one per frame)
        VkResult result = vkFreeDescriptorSets(
            device->logical_device,
            descriptor_pool,
            3,
            state->descriptor_set_state.descriptor_sets);
        if (result != VK_SUCCESS) {
            ERROR("Error freeing object shader descriptor sets!");
        }

        Platform::ZrMemory(state->descriptor_set_state.descriptor_states, sizeof(VulkanShaderDescriptorSetState) * VULKAN_SHADER_MAX_BINDINGS);

        if (state->instance_texture_maps.size()) {
            state->instance_texture_maps.clear();
        }

        //uniform_buffer->Free(state->offset);
        state->allocated_block->FreeBlock();
        state->offset = INVALID_ID;
        state->id = INVALID_ID;
    };

    b8 VulkanShader::SetUniform(ShaderUniformConfig* uniform, const void* value) {
        if (uniform->type == ShaderUniformType::SAMPLER) {
            if (uniform->scope == ShaderScope::GLOBAL) {
                global_texture_maps[uniform->location] = *(TextureMap*)value;
                return true;
            } 
            instance_states[bound_instance_id].instance_texture_maps[uniform->location] = (TextureMap*)value;
            return true;
        } 

        if (uniform->scope == ShaderScope::LOCAL) {
            VulkanRendererBackend* backend = VulkanRendererBackend::GetInstance();
            VulkanDevice* device = backend->GetVulkanDevice();
            u32 image_index = backend->GetImageIndex();
            VulkanCommandBuffer* command_buffer = backend->GetGraphicsCommandBufers()[image_index];

            vkCmdPushConstants(
                command_buffer->handle, 
                pipeline->pipeline_layout, 
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                uniform->offset, uniform->size, value);
            
            return true;
        } 
        
        u64 addr = (u64)mapped_uniform_buffer_block;
        addr += bound_ubo_offset + uniform->offset;
        Platform::CpMemory((void*)addr, value, uniform->size);

        return true;
    };
}