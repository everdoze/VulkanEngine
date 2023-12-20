#include "base_shader.hpp"

#include "core/utils/string.hpp"
#include "platform/platform.hpp"
#include "core/logger/logger.hpp"
#include "../helpers.hpp"
#include "../vulkan.hpp"
#include "systems/resource/resource_system.hpp"

namespace Engine {

    VulkanShader::VulkanShader(std::string name) {
        this->name = name;
    };

    VulkanShader::~VulkanShader() {
        this->name.clear();
    };

    b8 VulkanShader::CreateShaderStage(
        std::string name,
        std::string type,
        VkShaderStageFlagBits shader_stage_flag,
        VulkanShaderStage* out_shader_stage) {
        
        VulkanRendererBackend* backend = static_cast<VulkanRendererBackend*>(RendererFrontend::GetBackend());

        Platform::ZMemory(&out_shader_stage->create_info, sizeof(VkShaderModuleCreateInfo));

        std::string shader_name = StringFormat("%s.%s.spv", name.c_str(), type.c_str());
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

        Platform::ZMemory(&out_shader_stage->shader_stage_create_info, sizeof(VkPipelineShaderStageCreateInfo));
        out_shader_stage->shader_stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        out_shader_stage->shader_stage_create_info.stage = shader_stage_flag;
        out_shader_stage->shader_stage_create_info.module = out_shader_stage->handle;
        out_shader_stage->shader_stage_create_info.pName = "main";

        return true;
    };

}