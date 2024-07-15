#pragma once

#include "YUME/Core/base.h"
#include <vulkan/vulkan_core.h>



namespace YUME::Utils
{
	std::string GetMessageSeverityStr(VkDebugUtilsMessageSeverityFlagBitsEXT p_Severity);

	std::string GetMessageType(VkDebugUtilsMessageTypeFlagsEXT p_Type);
}