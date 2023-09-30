#pragma once

#include "core/logger/logger.hpp"
#include <vulkan/vulkan.h>


#define VK_CHECK(expr) ASSERT(expr == VK_SUCCESS);

b8 IsVulkanResultSuccess (VkResult result);

const char* VulkanResultString(VkResult result, b8 get_extended);