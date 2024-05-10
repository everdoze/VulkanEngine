#include "sampler.hpp"
#include "helpers.hpp"
#include "vulkan.hpp"
#include "core/logger/logger.hpp"

namespace Engine {

    VulkanSampler::VulkanSampler(SamplerCreateInfo info) : Sampler(info) {
        VulkanRendererBackend* backend = VulkanRendererBackend::GetInstance();

        VkSamplerCreateInfo sampler_create_info = {VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};
        
        sampler_create_info.magFilter = ConvertFilterMode(filter_magnify);
        sampler_create_info.minFilter = ConvertFilterMode(filter_minify);
        sampler_create_info.addressModeU = ConvertRepeatType(repeat_u);
        sampler_create_info.addressModeV = ConvertRepeatType(repeat_v);
        sampler_create_info.addressModeW = ConvertRepeatType(repeat_w);

        // TODO: make other params configurable
        sampler_create_info.anisotropyEnable = VK_TRUE;
        sampler_create_info.maxAnisotropy = 16;
        sampler_create_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        sampler_create_info.unnormalizedCoordinates = VK_FALSE;
        sampler_create_info.compareEnable = VK_FALSE;
        sampler_create_info.compareOp = VK_COMPARE_OP_ALWAYS;
        sampler_create_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        sampler_create_info.mipLodBias = 0.0f;
        sampler_create_info.minLod = 0.0f;
        sampler_create_info.maxLod = 0.0f;

        VkResult result = vkCreateSampler(
            backend->GetVulkanDevice()->logical_device,
            &sampler_create_info,
            backend->GetVulkanAllocator(),
            &sampler
        );
        if (!IsVulkanResultSuccess(result)) {
            ERROR("Error occured during creation of texture sampler: %s", VulkanResultString(result, true));
            return;
        }
    };

    VulkanSampler::~VulkanSampler() {
        VulkanRendererBackend* backend = VulkanRendererBackend::GetInstance();

        vkDestroySampler(
            backend->GetVulkanDevice()->logical_device,
            sampler,
            backend->GetVulkanAllocator()
        );

        this->sampler = 0;
    }

    VkSamplerAddressMode VulkanSampler::ConvertRepeatType(TextureRepeat repeat) {
        switch (repeat) {
            case TextureRepeat::REPEAT:
                return VK_SAMPLER_ADDRESS_MODE_REPEAT;
            
            case TextureRepeat::MIRRORED_REPEAT:
                return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;

            case TextureRepeat::CLAMP_TO_BORDER:
                return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;

            case TextureRepeat::CLAMP_TO_EDGE:
                return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;

            default:
                WARN("VulkanSampler::ConvertRepeatType: type '%x' is not supported.", repeat);
                return VK_SAMPLER_ADDRESS_MODE_REPEAT;
        }
    }

    VkFilter VulkanSampler::ConvertFilterMode(TextureFilterMode filter) {
        switch (filter) {
            case TextureFilterMode::LINEAR:
                return VK_FILTER_LINEAR;

            case TextureFilterMode::NEAREST:
                return VK_FILTER_NEAREST;

            default:
                WARN("VulkanSampler::ConvertFilterMode: type '%x' is not supported.", filter);
                return VK_FILTER_LINEAR;
        }
    };
};

