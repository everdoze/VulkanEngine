#pragma once

#include <vulkan/vulkan.h>
#include "defines.hpp"
#include "resources/texture/sampler.hpp"
#include "resources/texture/texture_types.hpp"

namespace Engine {

    class VulkanSampler : public Sampler {
        public:
            VulkanSampler(SamplerCreateInfo info);
            ~VulkanSampler() override;

            VkSampler& GetSampler() {
                return sampler;
            };

            VkSamplerAddressMode ConvertRepeatType(TextureRepeat repeat);
            VkFilter ConvertFilterMode(TextureFilterMode filter);

        protected:
            VkSampler sampler;
    };

};

