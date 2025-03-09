#include "pipeline.hpp"

#include "vulkan.hpp"
#include "platform/platform.hpp"
#include "renderer/renderer_types.hpp"
#include "helpers.hpp"

namespace Engine {

    VulkanPipeline::VulkanPipeline(
        VulkanRenderpass* renderpass,
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
        b8 depth_test_enabled) {
        
        VulkanRendererBackend* backend = VulkanRendererBackend::GetInstance();

        this->renderpass = renderpass;

        ready = false;

        //Viewport
        VkPipelineViewportStateCreateInfo viewport_state_create_info = {VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};
        viewport_state_create_info.viewportCount = 1;
        viewport_state_create_info.pViewports = &viewport;
        viewport_state_create_info.scissorCount = 1;
        viewport_state_create_info.pScissors = &scissor;

        //Rasterizer
        VkPipelineRasterizationStateCreateInfo rasterizer_create_info = {VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
        rasterizer_create_info.depthClampEnable = VK_FALSE;
        rasterizer_create_info.rasterizerDiscardEnable = VK_FALSE;
        rasterizer_create_info.polygonMode = is_wireframe ? VK_POLYGON_MODE_LINE : VK_POLYGON_MODE_FILL;
        rasterizer_create_info.lineWidth = 1.0f;
        rasterizer_create_info.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizer_create_info.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        rasterizer_create_info.depthBiasEnable = VK_FALSE;
        rasterizer_create_info.depthBiasConstantFactor = 0.0f;
        rasterizer_create_info.depthBiasClamp = 0.0f;
        rasterizer_create_info.depthBiasSlopeFactor = 0.0f;

        //Multisampling
        VkPipelineMultisampleStateCreateInfo multisampling_create_info = {VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
        multisampling_create_info.sampleShadingEnable = VK_FALSE;
        multisampling_create_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        multisampling_create_info.minSampleShading = 1.0f;
        multisampling_create_info.pSampleMask = 0;
        multisampling_create_info.alphaToCoverageEnable = VK_FALSE;
        multisampling_create_info.alphaToOneEnable = VK_FALSE;

        //Depth and stencil testing
        VkPipelineDepthStencilStateCreateInfo depth_stencil_create_info = {VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
        depth_stencil_create_info.depthTestEnable = depth_test_enabled ? VK_TRUE : VK_FALSE;
        depth_stencil_create_info.depthWriteEnable = depth_test_enabled ? VK_TRUE : VK_FALSE;
        depth_stencil_create_info.depthCompareOp = depth_test_enabled ? VK_COMPARE_OP_LESS : VK_COMPARE_OP_NEVER;
        depth_stencil_create_info.depthBoundsTestEnable = VK_FALSE;
        depth_stencil_create_info.stencilTestEnable = VK_FALSE;
        

        //Color blend attachment
        VkPipelineColorBlendAttachmentState color_blend_attachment_state;
        Platform::ZrMemory(&color_blend_attachment_state, sizeof(VkPipelineColorBlendAttachmentState));
        color_blend_attachment_state.blendEnable = VK_TRUE;
        color_blend_attachment_state.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        color_blend_attachment_state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        color_blend_attachment_state.colorBlendOp = VK_BLEND_OP_ADD;
        color_blend_attachment_state.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        color_blend_attachment_state.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        color_blend_attachment_state.alphaBlendOp = VK_BLEND_OP_ADD;
        color_blend_attachment_state.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

        // Color blend state
        VkPipelineColorBlendStateCreateInfo color_blend_state_create_info = {VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
        color_blend_state_create_info.logicOpEnable = VK_FALSE;
        color_blend_state_create_info.logicOp = VK_LOGIC_OP_COPY;
        color_blend_state_create_info.attachmentCount = 1;
        color_blend_state_create_info.pAttachments = &color_blend_attachment_state;

        //Dynamic state
        const u32 dynamic_state_count = 3;
        VkDynamicState dynamic_states[dynamic_state_count] = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR,
            VK_DYNAMIC_STATE_LINE_WIDTH
        };

        VkPipelineDynamicStateCreateInfo dynamic_state_create_info = {VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};
        dynamic_state_create_info.dynamicStateCount = dynamic_state_count;
        dynamic_state_create_info.pDynamicStates = dynamic_states;

        // Vertex input 
        VkVertexInputBindingDescription binding_description;
        Platform::ZrMemory(&binding_description, sizeof(VkVertexInputBindingDescription));
        binding_description.binding = 0;
        binding_description.stride = stride;
        binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        // Attibutes
        VkPipelineVertexInputStateCreateInfo vertex_input_create_info = {VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
        vertex_input_create_info.vertexBindingDescriptionCount = 1;
        vertex_input_create_info.pVertexBindingDescriptions = &binding_description;
        vertex_input_create_info.vertexAttributeDescriptionCount = attribute_count;
        vertex_input_create_info.pVertexAttributeDescriptions = attributes;

        // Input assembly
        VkPipelineInputAssemblyStateCreateInfo input_assembly_create_info = {VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
        input_assembly_create_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        input_assembly_create_info.primitiveRestartEnable = VK_FALSE;

        // Pipeline layout
        VkPipelineLayoutCreateInfo pipeline_layout_create_info = {VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};


        // Push constants
        if (push_constant_range_count > 0) {
            if (push_constant_range_count > 32) {
                ERROR("VulkanPipeline::VulkanPipeline - cannot have more than 32 push constant ranges. Passed count: %i", push_constant_range_count);
                return;
            }

            // NOTE: 32 is the max number of ranges we can ever have, since spec only guarantees 128 bytes with 4-byte alignment.
            VkPushConstantRange push_constants[32];
            Platform::ZrMemory(push_constants, sizeof(VkPushConstantRange) * 32);
            for (u32 i = 0; i < push_constant_range_count; ++i) {
                push_constants[i].offset = push_constant_ranges[i].offset;
                push_constants[i].size = push_constant_ranges[i].size;
                push_constants[i].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
                
            }
            pipeline_layout_create_info.pushConstantRangeCount = push_constant_range_count;
            pipeline_layout_create_info.pPushConstantRanges = push_constants;
        } else {
            pipeline_layout_create_info.pushConstantRangeCount = 0;
            pipeline_layout_create_info.pPushConstantRanges = nullptr;
        }

        pipeline_layout_create_info.setLayoutCount = descriptor_set_layout_count;
        pipeline_layout_create_info.pSetLayouts = descriptors_set_layouts;

        VK_CHECK(vkCreatePipelineLayout(
            backend->GetVulkanDevice()->logical_device,
            &pipeline_layout_create_info,
            backend->GetVulkanAllocator(),
            &this->pipeline_layout));

        // Pipeline
        VkGraphicsPipelineCreateInfo pipeline_create_info = {VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};
        pipeline_create_info.stageCount = stage_count;
        pipeline_create_info.pStages = stages;
        pipeline_create_info.pVertexInputState = &vertex_input_create_info;
        pipeline_create_info.pInputAssemblyState = &input_assembly_create_info;

        pipeline_create_info.pViewportState = &viewport_state_create_info;
        pipeline_create_info.pRasterizationState = &rasterizer_create_info;
        pipeline_create_info.pMultisampleState = &multisampling_create_info;
        pipeline_create_info.pDepthStencilState = &depth_stencil_create_info;
        pipeline_create_info.pColorBlendState = &color_blend_state_create_info;
        pipeline_create_info.pDynamicState = &dynamic_state_create_info;
        pipeline_create_info.pTessellationState = 0;

        pipeline_create_info.layout = this->pipeline_layout;

        pipeline_create_info.renderPass = renderpass->handle;
        pipeline_create_info.subpass = 0;
        pipeline_create_info.basePipelineHandle = VK_NULL_HANDLE;
        pipeline_create_info.basePipelineIndex = -1;

        VkResult result = vkCreateGraphicsPipelines(
            backend->GetVulkanDevice()->logical_device,
            VK_NULL_HANDLE,
            1,
            &pipeline_create_info,
            backend->GetVulkanAllocator(),
            &this->handle);

        if (IsVulkanResultSuccess(result)) {
            DEBUG("Graphics pipeline for renderpass '%s' created successfully.", renderpass->GetName().c_str());
            ready = true;
            return;
        }

        ERROR("vkCreateGraphicsPipelines failed with %s.", VulkanResultString(result, true));
    };

    VulkanPipeline::~VulkanPipeline() {
        VulkanRendererBackend* backend = VulkanRendererBackend::GetInstance();
        
        if (this->handle) {
            vkDestroyPipeline(
                backend->GetVulkanDevice()->logical_device, 
                this->handle, backend->GetVulkanAllocator());
            this->handle = nullptr;
        }

        if (this->pipeline_layout) {
            vkDestroyPipelineLayout(
                backend->GetVulkanDevice()->logical_device, 
                this->pipeline_layout, backend->GetVulkanAllocator());
            this->pipeline_layout = 0;
        }
    };

    void VulkanPipeline::Bind(VulkanCommandBuffer* command_buffer, VkPipelineBindPoint bind_point) {
        vkCmdBindPipeline(command_buffer->handle, bind_point, this->handle);
    };

};