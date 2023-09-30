#pragma once

#include <vulkan/vulkan.h>
#include "defines.hpp"

namespace Engine {

    class VulkanFence {
        public:
            VkFence handle;
            b8 is_signaled;

            VulkanFence(b8 create_signaled);

            ~VulkanFence();

            b8 Wait(u64 timeout_ns);

            void Reset();
    };

};