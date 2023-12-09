#include "material_shader.hpp"

#include "core/utils/string.hpp"
#include "platform/platform.hpp"
#include "platform/filesystem.hpp"
#include "core/logger/logger.hpp"
#include "../vulkan_helpers.hpp"
#include "../vulkan.hpp"
#include "../vulkan_texture.hpp"
#include "systems/texture/texture_system.hpp"

namespace Engine {

    VulkanShader::VulkanShader(std::string name) {
        VulkanRendererBackend* backend = static_cast<VulkanRendererBackend*>(RendererFrontend::GetBackend());

        this->ready = false;
        this->name = name;

        std::string shader_stages[VULKAN_MATERIAL_SHADER_STAGE_COUNT] = {"vert", "frag"};
        VkShaderStageFlagBits stage_types[VULKAN_MATERIAL_SHADER_STAGE_COUNT] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT};

        for (u32 i = 0; i < VULKAN_MATERIAL_SHADER_STAGE_COUNT; ++i) {
            if (!this->CreateShaderStage(name, shader_stages[i], stage_types[i], &this->stages[i])) {
                ERROR("Unable to create shader stage '%s' for '%s'", shader_stages[i].c_str(), name.c_str());
                return;
            }
        }

        // Global Descriptors
        VkDescriptorSetLayoutBinding global_ubo_layout_binding;
        global_ubo_layout_binding.binding = 0;
        global_ubo_layout_binding.descriptorCount = 1;
        global_ubo_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        global_ubo_layout_binding.pImmutableSamplers = 0;
        global_ubo_layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

        VkDescriptorSetLayoutCreateInfo global_layout_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
        global_layout_info.bindingCount = 1;
        global_layout_info.pBindings = &global_ubo_layout_binding;
        VK_CHECK(vkCreateDescriptorSetLayout(
            backend->GetVulkanDevice()->logical_device, 
            &global_layout_info, 
            backend->GetVulkanAllocator(), 
            &this->global_descriptor_set_layout));

        // Global descriptor pool: Used for global items such as view/projection matrix.
        VkDescriptorPoolSize global_pool_size;
        global_pool_size.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        global_pool_size.descriptorCount = backend->GetVulkanSwapchain()->image_count;

        VkDescriptorPoolCreateInfo global_pool_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
        global_pool_info.poolSizeCount = 1;
        global_pool_info.pPoolSizes = &global_pool_size;
        global_pool_info.maxSets = backend->GetVulkanSwapchain()->image_count;
        VK_CHECK(vkCreateDescriptorPool(
            backend->GetVulkanDevice()->logical_device, 
            &global_pool_info, 
            backend->GetVulkanAllocator(), 
            &this->global_descriptor_pool));

        // Local/Material descriptors
        const u32 local_sampler_count = 1;
        VkDescriptorType descriptor_types[VULKAN_MATERIAL_SHADER_DESCRIPTOR_COUNT] = {
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
        };
        VkDescriptorSetLayoutBinding bindings[VULKAN_MATERIAL_SHADER_DESCRIPTOR_COUNT];
        Platform::ZMemory(&bindings, sizeof(VkDescriptorSetLayoutBinding) * VULKAN_MATERIAL_SHADER_DESCRIPTOR_COUNT);
        for (u32 i = 0; i < VULKAN_MATERIAL_SHADER_DESCRIPTOR_COUNT; ++i) {
            bindings[i].binding = i;
            bindings[i].descriptorCount = 1;
            bindings[i].descriptorType = descriptor_types[i];
            bindings[i].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        }

        VkDescriptorSetLayoutCreateInfo layout_create_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
        layout_create_info.bindingCount = VULKAN_MATERIAL_SHADER_DESCRIPTOR_COUNT;
        layout_create_info.pBindings = bindings;
        VK_CHECK(vkCreateDescriptorSetLayout(
            backend->GetVulkanDevice()->logical_device,
            &layout_create_info,
            backend->GetVulkanAllocator(),
            &this->material_descriptor_set_layout
        ));

        // Local/Material descriptor pool: Used for material-specific items like diffuse color
        VkDescriptorPoolSize material_pool_sizes[2];
        material_pool_sizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        material_pool_sizes[0].descriptorCount = VULKAN_MATERIAL_MAX_OBJECT_COUNT;

        material_pool_sizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        material_pool_sizes[1].descriptorCount = local_sampler_count * VULKAN_MATERIAL_MAX_OBJECT_COUNT;

        VkDescriptorPoolCreateInfo descriptor_pool_create_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
        descriptor_pool_create_info.poolSizeCount = 1;
        descriptor_pool_create_info.pPoolSizes = material_pool_sizes;
        descriptor_pool_create_info.maxSets = VULKAN_MATERIAL_MAX_OBJECT_COUNT;
        VK_CHECK(vkCreateDescriptorPool(
            backend->GetVulkanDevice()->logical_device,
            &descriptor_pool_create_info,
            backend->GetVulkanAllocator(),
            &material_descriptor_pool
        ));

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

        // Attributes
        u32 offset = 0;
        const i32 attribute_count = 2;
        VkVertexInputAttributeDescription attribute_descriptions[attribute_count];

        // Position
        VkFormat formats[attribute_count] = {
            VK_FORMAT_R32G32B32_SFLOAT,
            VK_FORMAT_R32G32_SFLOAT
        };

        u64 sizes[attribute_count] = {
            sizeof(glm::vec3),
            sizeof(glm::vec2)
        };

        for (u32 i = 0; i < attribute_count; ++i) {
            attribute_descriptions[i].binding = 0;   // binding index - should match binding desc
            attribute_descriptions[i].location = i;  // attrib location
            attribute_descriptions[i].format = formats[i];
            attribute_descriptions[i].offset = offset;
            offset += sizes[i];
        }

        // Desciptor set layouts.
        const i32 descriptor_set_layout_count = 2;
        VkDescriptorSetLayout layouts[descriptor_set_layout_count] = {
            this->global_descriptor_set_layout,
            this->material_descriptor_set_layout
        };

        // Stages
        // NOTE: Should match the number of shader->stages.
        VkPipelineShaderStageCreateInfo stage_create_infos[VULKAN_MATERIAL_SHADER_STAGE_COUNT];
        Platform::ZMemory(stage_create_infos, sizeof(stage_create_infos));
        for (u32 i = 0; i < VULKAN_MATERIAL_SHADER_STAGE_COUNT; ++i) {
            stage_create_infos[i].sType = this->stages[i].shader_stage_create_info.sType;
            stage_create_infos[i] = this->stages[i].shader_stage_create_info;
        }

        this->pipeline = new VulkanPipeline(
            backend->GetMainRenderpass(),
            attribute_count,
            attribute_descriptions,
            descriptor_set_layout_count, layouts,
            VULKAN_MATERIAL_SHADER_STAGE_COUNT,
            stage_create_infos,
            viewport,
            scissor,
            false
        );

        if (!this->pipeline->ready) {
            ERROR("Failed to load graphics pipeline for object shader.");
            return;
        }

        u32 device_local_bit = backend->GetVulkanDevice()->supports_device_local_host_visible ? VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT : 0;
        this->global_uniform_buffer = new VulkanBuffer(
            sizeof(GlobalUniformObject), 
            (VkBufferUsageFlagBits)(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT),
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | device_local_bit,
            true); 
        
        if (!this->global_uniform_buffer->ready) {
            ERROR("Vulkan buffer creation failed for object shader.");
            return;
        }

        // Allocate global descriptor sets.
        VkDescriptorSetLayout global_layouts[3] = {
            this->global_descriptor_set_layout,
            this->global_descriptor_set_layout,
            this->global_descriptor_set_layout};

        VkDescriptorSetAllocateInfo alloc_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
        alloc_info.descriptorPool = this->global_descriptor_pool;
        alloc_info.descriptorSetCount = 3;
        alloc_info.pSetLayouts = global_layouts;
        VK_CHECK(vkAllocateDescriptorSets(
            backend->GetVulkanDevice()->logical_device, 
            &alloc_info, 
            this->global_descriptor_sets));

        this->material_uniform_buffer = new VulkanBuffer(
            sizeof(MaterialUniformObject), 
            (VkBufferUsageFlagBits)(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT),
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            true); 
        
        if (!this->material_uniform_buffer->ready) {
            ERROR("Vulkan buffer creation failed for object shader.");
            return;
        }
        
        this->ready = true;
    };

    VulkanShader::~VulkanShader() {
        VulkanRendererBackend* backend = static_cast<VulkanRendererBackend*>(RendererFrontend::GetBackend());

        // Destroy material descriptor pool.
        vkDestroyDescriptorPool(
            backend->GetVulkanDevice()->logical_device, 
            this->material_descriptor_pool, 
            backend->GetVulkanAllocator());

        // Destroy material set layouts.
        vkDestroyDescriptorSetLayout(
            backend->GetVulkanDevice()->logical_device, 
            this->material_descriptor_set_layout, 
            backend->GetVulkanAllocator());

        // Destroy material uniform buffer
        delete this->material_uniform_buffer;
        // Destroy global uniform buffer
        delete this->global_uniform_buffer;

        // Destroy pipeline
        if (this->pipeline) {
            delete this->pipeline;
        }
        
        // Destroy global descriptor pool.
        vkDestroyDescriptorPool(
            backend->GetVulkanDevice()->logical_device, 
            this->global_descriptor_pool, 
            backend->GetVulkanAllocator());

        // Destroy descriptor set layouts.
        vkDestroyDescriptorSetLayout(
            backend->GetVulkanDevice()->logical_device, 
            this->global_descriptor_set_layout, 
            backend->GetVulkanAllocator());

        // Destroy shader modules.
        for (u32 i = 0; i < VULKAN_MATERIAL_SHADER_STAGE_COUNT; ++i) {
            vkDestroyShaderModule(backend->GetVulkanDevice()->logical_device, this->stages[i].handle, backend->GetVulkanAllocator());
            this->stages[i].handle = 0;
        }

        this->ready = false;
        this->name.clear();
    };

    b8 VulkanShader::CreateShaderStage(
        std::string name,
        std::string type,
        VkShaderStageFlagBits shader_stage_flag,
        VulkanShaderStage* out_shader_stage) {
        
        VulkanRendererBackend* backend = static_cast<VulkanRendererBackend*>(RendererFrontend::GetBackend());
        
        std::string file_name = StringFormat("assets/shaders/%s.%s.spv", name.c_str(), type.c_str());

        Platform::ZMemory(&out_shader_stage->create_info, sizeof(VkShaderModuleCreateInfo));

        File* file = FileSystem::FileOpen(file_name, FileMode::READ, true);

        if (!file->IsReady()) {
            ERROR("Unable to read shader module: %s.", file_name.c_str());
            return false;
        }

        std::vector<c8> bytes = file->ReadAllBytes();

        if (bytes.size() == 0) {
            ERROR("Unable to binary read shader module: %s.", file_name.c_str());
            return false;
        }

        FileSystem::FileClose(file);

        out_shader_stage->create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        out_shader_stage->create_info.codeSize = bytes.size();
        out_shader_stage->create_info.pCode = (u32*)bytes.data();

        VK_CHECK(vkCreateShaderModule(
            backend->GetVulkanDevice()->logical_device,
            &out_shader_stage->create_info,
            backend->GetVulkanAllocator(),
            &out_shader_stage->handle));

        Platform::ZMemory(&out_shader_stage->shader_stage_create_info, sizeof(VkPipelineShaderStageCreateInfo));
        out_shader_stage->shader_stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        out_shader_stage->shader_stage_create_info.stage = shader_stage_flag;
        out_shader_stage->shader_stage_create_info.module = out_shader_stage->handle;
        out_shader_stage->shader_stage_create_info.pName = "main";

        return true;
    };

    void VulkanShader::Use() {
        VulkanRendererBackend* backend = static_cast<VulkanRendererBackend*>(RendererFrontend::GetBackend());
        u32 image_index = backend->GetImageIndex();
        VulkanCommandBuffer* command_buffer = backend->GetGraphicsCommandBufers()[image_index];
        this->pipeline->Bind(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS);
    };

    void VulkanShader::UpdateGlobalState() {
        VulkanRendererBackend* backend = static_cast<VulkanRendererBackend*>(RendererFrontend::GetBackend());
        u32 image_index = backend->GetImageIndex();
        VkCommandBuffer command_buffer = backend->GetGraphicsCommandBufers()[image_index]->handle;
        VkDescriptorSet global_descriptor = this->global_descriptor_sets[image_index];

        // Bind the global descriptor set to be updated.
        vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->pipeline->pipeline_layout, 0, 1, &global_descriptor, 0, 0);

        // Configure the descriptors for the given index.
        u32 range = sizeof(GlobalUniformObject);
        u64 offset = 0;

        this->global_uniform_buffer->LoadData(offset, range, 0, &this->global_ubo);

        VkDescriptorBufferInfo bufferInfo;
        bufferInfo.buffer = this->global_uniform_buffer->handle;
        bufferInfo.offset = offset;
        bufferInfo.range = range;

        // Update descriptor sets.
        VkWriteDescriptorSet descriptor_write = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
        descriptor_write.dstSet = this->global_descriptor_sets[image_index];
        descriptor_write.dstBinding = 0;
        descriptor_write.dstArrayElement = 0;
        descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptor_write.descriptorCount = 1;
        descriptor_write.pBufferInfo = &bufferInfo;

        vkUpdateDescriptorSets(
            backend->GetVulkanDevice()->logical_device, 1, &descriptor_write, 0, 0);

    };

    void VulkanShader::UseModel(glm::mat4 model) {
        VulkanRendererBackend* backend = static_cast<VulkanRendererBackend*>(RendererFrontend::GetBackend());
        u32 image_index = backend->GetImageIndex();
        VkCommandBuffer command_buffer = backend->GetGraphicsCommandBufers()[image_index]->handle;

        vkCmdPushConstants(command_buffer, this->pipeline->pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &model);
    };

    void VulkanShader::UseMaterial(Material* material) {
        VulkanRendererBackend* backend = static_cast<VulkanRendererBackend*>(RendererFrontend::GetBackend());
        u32 image_index = backend->GetImageIndex();
        VkCommandBuffer command_buffer = backend->GetGraphicsCommandBufers()[image_index]->handle;

        // Get material state
        VulkanMaterialShaderInstanceState* material_state = &instance_states[material->GetInternalId()];
        VkDescriptorSet material_descriptor_set = material_state->descriptor_sets[image_index];

        // TODO: if needs update
        VkWriteDescriptorSet descriptor_writes[VULKAN_MATERIAL_SHADER_DESCRIPTOR_COUNT];
        Platform::ZMemory(descriptor_writes, sizeof(VkWriteDescriptorSet) * VULKAN_MATERIAL_SHADER_DESCRIPTOR_COUNT);
        u32 descriptor_count = 0;
        u32 descriptor_index = 0;

        // Descriptor 0 - Uniform buffer
        u32 range = sizeof(MaterialUniformObject);
        u64 offset = sizeof(MaterialUniformObject) * material->GetInternalId();
        MaterialUniformObject mbo;

        accumulator += backend->GetDeltaTime();
        f32 s = (glm::sin(accumulator) + 1.0f) / 2.0f;
        mbo.diffuse_color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

        this->material_uniform_buffer->LoadData(offset, range, 0, &mbo);

        // Only if descriptor has not been updated
        if (material_state->descriptor_states[descriptor_index].generations[image_index] == INVALID_ID) {
            VkDescriptorBufferInfo buffer_info;
            buffer_info.buffer = material_uniform_buffer->handle;
            buffer_info.offset = offset;
            buffer_info.range = range;

            VkWriteDescriptorSet descriptor = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
            descriptor.dstSet = material_descriptor_set;
            descriptor.dstBinding = descriptor_index;
            descriptor.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptor.descriptorCount = 1;
            descriptor.pBufferInfo = &buffer_info;
            
            descriptor_writes[descriptor_count] = descriptor;
            descriptor_count++;

            material_state->descriptor_states[descriptor_index].generations[image_index] = 1;
        }
        descriptor_index++;

        const u32 sampler_count = 1;
        VkDescriptorImageInfo image_infos[sampler_count];
        for (u32 i = 0; i < sampler_count; ++i) {
            VulkanTexture* t = static_cast<VulkanTexture*>(material->GetDiffuseMap().texture);
            u32* descriptor_generation = &material_state->descriptor_states[descriptor_index].generations[image_index];

            if (!t) {
                t = static_cast<VulkanTexture*>(TextureSystem::GetInstance()->GetDefaultTexture());
            }

            // Check if the descriptor needs updating first
            if (t && (*descriptor_generation != t->GetGeneration() || *descriptor_generation == INVALID_ID)) {
                image_infos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                image_infos[i].imageView = t->GetImage()->view;
                image_infos[i].sampler = t->GetSampler();

                VkWriteDescriptorSet descriptor = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
                descriptor.dstSet = material_descriptor_set;
                descriptor.dstBinding = descriptor_index;
                descriptor.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                descriptor.descriptorCount = 1;
                descriptor.pImageInfo = &image_infos[i];
                
                descriptor_writes[descriptor_count] = descriptor;
                descriptor_count++;

                if (t->GetGeneration() != INVALID_ID) {
                    *descriptor_generation = t->GetGeneration();
                }
                descriptor_index++;
            }
        }

        if (descriptor_count > 0) {
            vkUpdateDescriptorSets(
                backend->GetVulkanDevice()->logical_device,
                descriptor_count, descriptor_writes,
                0, 0
            );
        }

        vkCmdBindDescriptorSets(
            command_buffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            pipeline->pipeline_layout,
            1, 1, 
            &material_descriptor_set,
            0, 0
        );
    };

    b8 VulkanShader::AcquireResources(Material* material) {
        VulkanRendererBackend* backend = static_cast<VulkanRendererBackend*>(RendererFrontend::GetBackend());
        u32 material_id = instance_states.size();
        material->SetInternalId(material_id);

        VulkanMaterialShaderInstanceState instance_state;
        for (u32 i = 0; i < VULKAN_MATERIAL_SHADER_DESCRIPTOR_COUNT; ++i) {
            for (u32 j = 0; j < 3; ++j) {
                instance_state.descriptor_states[i].generations[j] = INVALID_ID;
            }
        }

        // Allocate descriptors sets
        VkDescriptorSetLayout layouts[3] = {
            material_descriptor_set_layout,
            material_descriptor_set_layout,
            material_descriptor_set_layout
        };

        VkDescriptorSetAllocateInfo allocate_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
        allocate_info.descriptorPool = material_descriptor_pool;
        allocate_info.descriptorSetCount = 3;
        allocate_info.pSetLayouts = layouts;
        VkResult result = vkAllocateDescriptorSets(
            backend->GetVulkanDevice()->logical_device,
            &allocate_info, instance_state.descriptor_sets
        );
        if (result != VK_SUCCESS) {
            ERROR("Error during allocation of descriptor sets in shader: %s.", name.c_str());
            return false;
        }

        instance_states.push_back(instance_state);

        return true;
    };
    
    void VulkanShader::ReleaseResources(u32 material_id) {
        VulkanRendererBackend* backend = static_cast<VulkanRendererBackend*>(RendererFrontend::GetBackend());
        VulkanMaterialShaderInstanceState* instance_state = &instance_states[material_id];

        const u32 descriptor_set_count = 3;

        vkDeviceWaitIdle(backend->GetVulkanDevice()->logical_device);

        VkResult result = vkFreeDescriptorSets(
            backend->GetVulkanDevice()->logical_device,
            material_descriptor_pool,
            descriptor_set_count,
            instance_state->descriptor_sets
        );
        if (result != VK_SUCCESS) {
            ERROR("Error during freeing descriptor sets in shader: %s", name.c_str());
        }

        for (u32 i = 0; i < VULKAN_MATERIAL_SHADER_DESCRIPTOR_COUNT; ++i) {
            for (u32 j = 0; j < 3; ++j) {
                instance_state->descriptor_states[i].generations[j] = INVALID_ID;
            }
        }

    };

};