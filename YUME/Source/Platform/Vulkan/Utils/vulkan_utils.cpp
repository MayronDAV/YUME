#include "YUME/yumepch.h"
#include "vulkan_utils.h"


namespace YUME::Utils
{
	std::string GetMessageSeverityStr(VkDebugUtilsMessageSeverityFlagBitsEXT p_Severity)
	{
		switch (p_Severity)
		{
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:		 return "Verbose";
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:			 return "Info";
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:		 return "Warning";
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:			 return "Error";
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_FLAG_BITS_MAX_ENUM_EXT: return "Critical";
			default:
				YM_CORE_ERROR("Unknown Message Severity!")
				return "Trace";
		}
	}

	std::string GetMessageType(VkDebugUtilsMessageTypeFlagsEXT p_Type)
	{
		switch (p_Type)
		{
			case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT:				 return "General";
			case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT:			 return "Validation";
			case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT:			 return "Performance";

	#ifdef _WIN64 // doesn't work on my Linux for some reason
			case VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT: return "Device address binding";
	#endif
			default:
				YM_CORE_ERROR("Unknown Message Type!")
				return "General";
		}
	}

	VkPolygonMode PolygonModeToVk(PolygonMode p_Mode)
	{
		switch (p_Mode)
		{
			case YUME::PolygonMode::FILL:  return VK_POLYGON_MODE_FILL;
			case YUME::PolygonMode::LINE:  return VK_POLYGON_MODE_LINE;
			case YUME::PolygonMode::POINT: return VK_POLYGON_MODE_POINT;
			default:
				YM_CORE_ERROR("Unknown Polygon Mode!")
				return VK_POLYGON_MODE_FILL;
		}
	}

	VkPrimitiveTopology DrawTypeToVk(DrawType p_Type)
	{
		switch (p_Type)
		{
			case YUME::DrawType::TRIANGLE: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
			case YUME::DrawType::LINES:    return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
			case YUME::DrawType::POINT:    return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
			default:
				YM_CORE_ERROR("Unknown Draw Type!")
				return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		}
	}

	VkFrontFace FrontFaceToVk(FrontFace p_Face)
	{
		switch (p_Face)
		{
			case YUME::FrontFace::CLOCKWISE:		 return VK_FRONT_FACE_CLOCKWISE;
			case YUME::FrontFace::COUNTER_CLOCKWISE: return VK_FRONT_FACE_COUNTER_CLOCKWISE;
			default:
				YM_CORE_ERROR("Unknown Front Face!")
				return VK_FRONT_FACE_CLOCKWISE;
		}
	}

	VkCullModeFlags CullModeToVk(CullMode p_Mode)
	{
		switch (p_Mode)
		{
			case CullMode::BACK:		 return VK_CULL_MODE_BACK_BIT;
			case CullMode::FRONT:		 return VK_CULL_MODE_FRONT_BIT;
			case CullMode::FRONTANDBACK: return VK_CULL_MODE_FRONT_AND_BACK;
			case CullMode::NONE:		 return VK_CULL_MODE_NONE;
			default:
				YM_CORE_ERROR("Unknown Cull Mode!")
				return VK_CULL_MODE_BACK_BIT;
		}
	}

}

