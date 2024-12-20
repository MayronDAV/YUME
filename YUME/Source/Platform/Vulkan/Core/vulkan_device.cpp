#include "YUME/yumepch.h"
#include "vulkan_device.h"
#include "YUME/Utils/utils.h"
#include "Platform/Vulkan/Renderer/vulkan_context.h"


namespace YUME
{

	std::vector<VkDeviceQueueCreateInfo> ConsolidateQueueCreateInfos(const std::vector<VkDeviceQueueCreateInfo>& p_OriginalQueueInfos)
	{
		YM_PROFILE_FUNCTION()

		std::unordered_map<uint32_t, VkDeviceQueueCreateInfo> queueInfoMap;
		for (const auto& info : p_OriginalQueueInfos)
		{
			auto it = queueInfoMap.find(info.queueFamilyIndex);
			if (it != queueInfoMap.end())
			{
				auto& existingInfo = it->second;

				std::vector<float> combinedPriorities(existingInfo.pQueuePriorities, existingInfo.pQueuePriorities + existingInfo.queueCount);
				combinedPriorities.insert(combinedPriorities.end(), info.pQueuePriorities, info.pQueuePriorities + info.queueCount);
				std::sort(combinedPriorities.begin(), combinedPriorities.end());
				combinedPriorities.erase(std::unique(combinedPriorities.begin(), combinedPriorities.end()), combinedPriorities.end());

				existingInfo.queueCount = static_cast<uint32_t>(combinedPriorities.size());
			}
			else
			{
				queueInfoMap[info.queueFamilyIndex] = info;
			}
		}

		std::vector<VkDeviceQueueCreateInfo> consolidatedQueueInfos;
		for (const auto& pair : queueInfoMap)
		{
			consolidatedQueueInfos.push_back(pair.second);
		}

		return consolidatedQueueInfos;
	}

	/////////////////////////////////////////////////////////////
	//////////////////// Physical Device ////////////////////////
	/////////////////////////////////////////////////////////////

	VulkanPhysicalDevice::VulkanPhysicalDevice()
	{
		YM_PROFILE_FUNCTION()

		YM_CORE_TRACE(VULKAN_PREFIX "Getting gpus...")

		auto instance = VulkanContext::GetInstance();

		auto res = vkEnumeratePhysicalDevices(instance, &m_GPUCount, nullptr);
		YM_CORE_VERIFY(res == VK_SUCCESS)
		YM_CORE_VERIFY(m_GPUCount > 0)

		std::vector<VkPhysicalDevice> devices(m_GPUCount);
		res = vkEnumeratePhysicalDevices(instance, &m_GPUCount, devices.data());
		YM_CORE_VERIFY(res == VK_SUCCESS)
		YM_CORE_VERIFY(!devices.empty())

		m_PhysicalDevices.resize(m_GPUCount);

		YM_CORE_INFO("Devices founded: ")
		for (uint32_t i = 0; i < m_GPUCount; i++)
		{

			VkPhysicalDevice device = devices[i];
			m_PhysicalDevices[i].Handle = device;

			vkGetPhysicalDeviceProperties(device, &m_PhysicalDevices[i].Properties);

			vkGetPhysicalDeviceFeatures(device, &m_PhysicalDevices[i].Features);

			uint32_t familyPropsCount = 0;
			vkGetPhysicalDeviceQueueFamilyProperties(device, &familyPropsCount, nullptr);

			m_PhysicalDevices[i].FamilyProperties.resize(familyPropsCount);
			vkGetPhysicalDeviceQueueFamilyProperties(device, &familyPropsCount, m_PhysicalDevices[i].FamilyProperties.data());


			vkGetPhysicalDeviceMemoryProperties(device, &m_PhysicalDevices[i].MemoryProperties);

			uint32_t extensionCount = 0;
			auto& physDevice = m_PhysicalDevices[i];
			res = vkEnumerateDeviceExtensionProperties(physDevice.Handle, nullptr, &extensionCount, nullptr);
			YM_CORE_VERIFY(res == VK_SUCCESS)
			YM_CORE_VERIFY(extensionCount > 0)

			physDevice.SupportedExtensions.resize(extensionCount);
			res = vkEnumerateDeviceExtensionProperties(physDevice.Handle, nullptr, &extensionCount, physDevice.SupportedExtensions.data());
			YM_CORE_VERIFY(res == VK_SUCCESS)

			for (uint32_t j = 0; j < m_PhysicalDevices[i].FamilyProperties.size(); j++)
			{
				static const float defaultQueuePriority(0.0f);
				auto familyProps = m_PhysicalDevices[i].FamilyProperties[j];

				if (familyProps.queueFlags & VK_QUEUE_COMPUTE_BIT)
				{
					m_PhysicalDevices[i].Indices.Compute = j;

					VkDeviceQueueCreateInfo queueInfo = {};
					queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
					queueInfo.queueFamilyIndex = j;
					queueInfo.queueCount = 1;
					queueInfo.pQueuePriorities = &defaultQueuePriority;
					m_PhysicalDevices[i].QueueCreateInfos.push_back(queueInfo);

					physDevice.SupportCompute = true;
				}
				
				if (familyProps.queueFlags & VK_QUEUE_TRANSFER_BIT)
				{
					m_PhysicalDevices[i].Indices.Transfer = j;

					VkDeviceQueueCreateInfo queueInfo = {};
					queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
					queueInfo.queueFamilyIndex = j;
					queueInfo.queueCount = 1;
					queueInfo.pQueuePriorities = &defaultQueuePriority;
					m_PhysicalDevices[i].QueueCreateInfos.push_back(queueInfo);
				}

				if (familyProps.queueFlags & VK_QUEUE_GRAPHICS_BIT)
				{
					m_PhysicalDevices[i].Indices.Graphics = j;

					VkDeviceQueueCreateInfo queueInfo = {};
					queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
					queueInfo.queueFamilyIndex = j;
					queueInfo.queueCount = 1;
					queueInfo.pQueuePriorities = &defaultQueuePriority;
					m_PhysicalDevices[i].QueueCreateInfos.push_back(queueInfo);
				}
			}

			m_PhysicalDevices[i].Info = GetInfo(physDevice.Handle);

			auto apiVersion = m_PhysicalDevices[i].Properties.apiVersion;
			YM_CORE_INFO("Vulkan : {0}.{1}.{2}", VK_API_VERSION_MAJOR(apiVersion), VK_API_VERSION_MINOR(apiVersion), VK_API_VERSION_PATCH(apiVersion))
			YM_CORE_INFO("GPU : {0}", m_PhysicalDevices[i].Info.Name)
			YM_CORE_INFO("Memory : {0} mb", m_PhysicalDevices[i].Info.Memory)
			YM_CORE_INFO("Vendor : {0}", m_PhysicalDevices[i].Info.Vendor)
			YM_CORE_INFO("Vendor ID : {0}", m_PhysicalDevices[i].Info.VendorID)
			YM_CORE_INFO("Driver Version : {0}", m_PhysicalDevices[i].Info.Driver)
			YM_CORE_INFO("APi Version : {0}", m_PhysicalDevices[i].Info.APIVersion)
		}

		YM_CORE_VERIFY(!m_PhysicalDevices.empty())
	}

	bool VulkanPhysicalDevice::IsExtensionSupported(const char* p_Extension)
	{
		for (const auto& extension : m_PhysicalDevices[m_SelectedIndex].SupportedExtensions)
		{
			if (strcmp(p_Extension, extension.extensionName))
				return true;
		}

		return false;
	}

	uint32_t VulkanPhysicalDevice::FindMemoryType(uint32_t p_TypeFilter, VkMemoryPropertyFlags p_Properties) const
	{
		auto selected = m_PhysicalDevices[m_SelectedIndex];
		for (uint32_t i = 0; i < selected.MemoryProperties.memoryTypeCount; i++) {
			if ((p_TypeFilter & (1 << i)) && (selected.MemoryProperties.memoryTypes[i].propertyFlags & p_Properties) == p_Properties) {
				return i;
			}
		}

		YM_CORE_ASSERT(false, "Failed to find suitable memory type!")
		return 0;
	}

	PhysicalDeviceInfo VulkanPhysicalDevice::GetInfo(VkPhysicalDevice p_Device) const
	{
		VkPhysicalDeviceProperties properties = {};
		vkGetPhysicalDeviceProperties(p_Device, &properties);

		VkPhysicalDeviceMemoryProperties memoryProperties = {};
		vkGetPhysicalDeviceMemoryProperties(p_Device, &memoryProperties);


		uint64_t memory = memoryProperties.memoryHeaps[0].size;
		auto memoryMB = static_cast<uint32_t>(memory / 1024 / 1024);

		PhysicalDeviceInfo info = {};
		info.Name = std::string(properties.deviceName);
		info.VendorID = properties.vendorID;
		info.Vendor = info.GetVendorName();
		info.Memory = memoryMB;
		info.Driver = info.DecodeVersion(uint32_t(properties.driverVersion));
		info.APIVersion = info.DecodeVersion(uint32_t(properties.apiVersion));
		info.Type = properties.deviceType;

		return info;
	}


	std::string PhysicalDeviceInfo::GetVendorName() const
	{
		std::string name = "Unknown";

		if (VendorID == 0x10DE || Name == "Nvidia")
		{
			name = "Nvidia";
		}
		else if (VendorID == 0x1002 || VendorID == 0x1022 || Name == "Amd")
		{
			name = "AMD";
		}
		else if (VendorID == 0x8086 || VendorID == 0x163C || VendorID == 0x8087 || Name == "Intel")
		{
			name = "Intel";
		}
		else if (VendorID == 0x13B5 || Name == "Arm")
		{
			name = "Arm";
		}
		else if (VendorID == 0x5143 || Name == "Qualcomm")
		{
			name = "Qualcomm";
		}
		else if (VendorID == 0x106b || Name == "Apple")
		{
			return "Apple";
		}

		return name;
	}

	std::string PhysicalDeviceInfo::DecodeVersion(const uint32_t p_Version) const
	{
		std::vector<char> buffer;
		buffer.resize(256);

		if (Vendor == "Nvidia")
		{
			std::format_to(
				buffer.data(),
				"{}.{}.{}.{}",
				(p_Version >> 22) & 0x3ff,
				(p_Version >> 14) & 0x0ff,
				(p_Version >> 6) & 0x0ff,
				p_Version & 0x003f);
		}
#if YM_PLATFORM_WINDOWS
		else if (Vendor == "Intel")
		{
			std::format_to(
				buffer.data(),
				"{}.{}",
				(p_Version >> 14),
				p_Version & 0x3fff);
		}
#endif
		else // Vulkan version conventions
		{
			std::format_to(
				buffer.data(),
				"{}.{}.{}",
				(p_Version >> 22),
				(p_Version >> 12) & 0x3ff,
				p_Version & 0xfff);
		}

		return buffer.data();
	}


	/////////////////////////////////////////////////////////////
	////////////////////////// Device ///////////////////////////
	/////////////////////////////////////////////////////////////

	VulkanDevice::~VulkanDevice()
	{
		YM_PROFILE_FUNCTION()

		m_CommandPool.reset();
		m_DescriptorPool->Destroy();

		{
			YM_CORE_TRACE(VULKAN_PREFIX "Saving pipeline cache...")
			size_t cacheSize;
			vkGetPipelineCacheData(m_Device, m_PipelineCache, &cacheSize, nullptr);

			std::vector<uint8_t> cacheData(cacheSize);
			vkGetPipelineCacheData(m_Device, m_PipelineCache, &cacheSize, cacheData.data());

			std::ofstream cacheFile(m_PipelineCachePath, std::ios::binary);
			cacheFile.write(reinterpret_cast<char*>(cacheData.data()), cacheData.size());
			cacheFile.close();
		}

		#ifdef USE_VMA_ALLOCATOR
			for (auto& pool : m_SmallAllocPools)
			{
				vmaDestroyPool(m_Allocator, pool.second);
			}
			m_SmallAllocPools.clear();

			vmaDestroyAllocator(m_Allocator);
		#endif

		YM_CORE_TRACE(VULKAN_PREFIX "Destroying pipeline cache...")
		vkDestroyPipelineCache(m_Device, m_PipelineCache, VK_NULL_HANDLE);

		YM_CORE_TRACE(VULKAN_PREFIX "Destroying device...")
		vkDestroyDevice(m_Device, VK_NULL_HANDLE);
	}

	void VulkanDevice::Init()
	{
		YM_PROFILE_FUNCTION()

		YM_CORE_TRACE(VULKAN_PREFIX "Creating device...")

		m_PhysicalDevice = CreateUnique<VulkanPhysicalDevice>();

		std::vector<const char*> devExts = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME,
			VK_KHR_SHADER_DRAW_PARAMETERS_EXTENSION_NAME,
			VK_EXT_CUSTOM_BORDER_COLOR_EXTENSION_NAME,
			VK_EXT_SHADER_ATOMIC_FLOAT_EXTENSION_NAME
		};
		std::vector<const char*> requiredExts;

		YM_CORE_TRACE(VULKAN_PREFIX "Checking device extensions...")
		for (auto ext : devExts)
		{
			if (m_PhysicalDevice->IsExtensionSupported(ext)) 
			{
				YM_CORE_INFO(VULKAN_PREFIX "Device extension {} founded!", ext)
				requiredExts.push_back(ext);
			}
			else
			{
				YM_CORE_ERROR(VULKAN_PREFIX "Device extension {} not founded!", ext)
			}
		}
		std::cout << "\n";

		auto physDevice = m_PhysicalDevice->Selected();

		// TEMPORARY ?
		YM_CORE_VERIFY(physDevice.Features.independentBlend == VK_TRUE, "The feature independentBlend isn't supported :c")

		VkPhysicalDeviceFeatures physFeatures{ 0 };
		physFeatures.geometryShader		= physDevice.Features.geometryShader;
		physFeatures.tessellationShader = physDevice.Features.tessellationShader;
		physFeatures.samplerAnisotropy  = physDevice.Features.samplerAnisotropy;
		physFeatures.wideLines			= physDevice.Features.wideLines;
		physFeatures.fillModeNonSolid	= physDevice.Features.fillModeNonSolid;
		physFeatures.independentBlend	= VK_TRUE;
		physFeatures.depthBiasClamp		= VK_TRUE;
		physFeatures.depthClamp			= VK_TRUE;

		auto queueCreateInfos = ConsolidateQueueCreateInfos(physDevice.QueueCreateInfos);


		VkPhysicalDeviceCustomBorderColorFeaturesEXT customBorderColorFeatures{};
		customBorderColorFeatures.sType				 = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CUSTOM_BORDER_COLOR_FEATURES_EXT;
		customBorderColorFeatures.customBorderColors = VK_TRUE;

		VkDeviceCreateInfo deviceCreateInfo			 = {};
		deviceCreateInfo.pNext						 = &customBorderColorFeatures;
		deviceCreateInfo.sType						 = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		deviceCreateInfo.queueCreateInfoCount		 = (uint32_t)queueCreateInfos.size();
		deviceCreateInfo.pQueueCreateInfos			 = queueCreateInfos.data();
		deviceCreateInfo.enabledExtensionCount		 = (uint32_t)requiredExts.size();
		deviceCreateInfo.ppEnabledExtensionNames	 = requiredExts.data();
		deviceCreateInfo.pEnabledFeatures			 = &physFeatures;
		deviceCreateInfo.enabledLayerCount			 = 0;

		auto res									 = vkCreateDevice(physDevice.Handle, &deviceCreateInfo, VK_NULL_HANDLE, &m_Device);
		YM_CORE_VERIFY(res == VK_SUCCESS)

		vkGetDeviceQueue(m_Device, physDevice.Indices.Graphics, 0, &m_GraphicQueue);
		vkGetDeviceQueue(m_Device, physDevice.Indices.Compute, 0, &m_ComputeQueue);
		vkGetDeviceQueue(m_Device, physDevice.Indices.Transfer, 0, &m_TransferQueue);

#ifdef USE_VMA_ALLOCATOR
		YM_CORE_TRACE(VULKAN_PREFIX "Creating vma...")

		VmaAllocatorCreateInfo allocatorInfo		= {};
		allocatorInfo.physicalDevice				= GetPhysicalDevice();
		allocatorInfo.device						= m_Device;
		allocatorInfo.instance						= VulkanContext::GetInstance();
		allocatorInfo.vulkanApiVersion				= VK_API_VERSION_1_3;

		VmaVulkanFunctions fn						= {};
		fn.vkAllocateMemory							= (PFN_vkAllocateMemory)vkAllocateMemory;
		fn.vkBindBufferMemory						= (PFN_vkBindBufferMemory)vkBindBufferMemory;
		fn.vkBindImageMemory						= (PFN_vkBindImageMemory)vkBindImageMemory;
		fn.vkCmdCopyBuffer							= (PFN_vkCmdCopyBuffer)vkCmdCopyBuffer;
		fn.vkCreateBuffer							= (PFN_vkCreateBuffer)vkCreateBuffer;
		fn.vkCreateImage							= (PFN_vkCreateImage)vkCreateImage;
		fn.vkDestroyBuffer							= (PFN_vkDestroyBuffer)vkDestroyBuffer;
		fn.vkDestroyImage							= (PFN_vkDestroyImage)vkDestroyImage;
		fn.vkFlushMappedMemoryRanges				= (PFN_vkFlushMappedMemoryRanges)vkFlushMappedMemoryRanges;
		fn.vkFreeMemory								= (PFN_vkFreeMemory)vkFreeMemory;
		fn.vkGetBufferMemoryRequirements			= (PFN_vkGetBufferMemoryRequirements)vkGetBufferMemoryRequirements;
		fn.vkGetImageMemoryRequirements				= (PFN_vkGetImageMemoryRequirements)vkGetImageMemoryRequirements;
		fn.vkGetPhysicalDeviceMemoryProperties		= (PFN_vkGetPhysicalDeviceMemoryProperties)vkGetPhysicalDeviceMemoryProperties;
		fn.vkGetPhysicalDeviceProperties			= (PFN_vkGetPhysicalDeviceProperties)vkGetPhysicalDeviceProperties;
		fn.vkInvalidateMappedMemoryRanges			= (PFN_vkInvalidateMappedMemoryRanges)vkInvalidateMappedMemoryRanges;
		fn.vkMapMemory								= (PFN_vkMapMemory)vkMapMemory;
		fn.vkUnmapMemory							= (PFN_vkUnmapMemory)vkUnmapMemory;
		fn.vkGetDeviceImageMemoryRequirements		= (PFN_vkGetDeviceImageMemoryRequirements)vkGetDeviceImageMemoryRequirements;
		fn.vkGetDeviceBufferMemoryRequirements		= (PFN_vkGetDeviceBufferMemoryRequirements)vkGetDeviceBufferMemoryRequirements;
		fn.vkBindImageMemory2KHR					= nullptr;
		fn.vkBindBufferMemory2KHR					= nullptr;
		fn.vkGetPhysicalDeviceMemoryProperties2KHR  = nullptr;
		fn.vkGetImageMemoryRequirements2KHR			= nullptr;
		fn.vkGetBufferMemoryRequirements2KHR		= nullptr;
		fn.vkGetInstanceProcAddr					= (PFN_vkGetInstanceProcAddr)vkGetInstanceProcAddr;
		fn.vkGetDeviceProcAddr						= (PFN_vkGetDeviceProcAddr)vkGetDeviceProcAddr;
		allocatorInfo.pVulkanFunctions				= &fn;
		allocatorInfo.preferredLargeHeapBlockSize   = 64 * 1024 * 1024;

		auto result									= vmaCreateAllocator(&allocatorInfo, &m_Allocator);
		YM_CORE_VERIFY(result == VK_SUCCESS)
#endif

		m_CommandPool								 = CreateRef<VulkanCommandPool>(physDevice.Indices.Graphics, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
		m_DescriptorPool							 = CreateRef<VulkanDescriptorPool>();
		m_DescriptorPool->Init(100, VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT);

		Utils::CreateDirectoryIfNeeded(m_PipelineCacheDir);

		std::ifstream cacheFile(m_PipelineCachePath, std::ios::binary | std::ios::ate);
		if (cacheFile.is_open())
		{
			YM_CORE_TRACE(VULKAN_PREFIX "Loading pipeline cache file...")

			auto cacheSize = (size_t)cacheFile.tellg();
			cacheFile.seekg(0, std::ios::beg);

			std::vector<uint8_t> cacheData(cacheSize);
			cacheFile.read(reinterpret_cast<char*>(cacheData.data()), cacheSize);
			cacheFile.close();

			VkPipelineCacheCreateInfo pipelineCacheCI = {};
			pipelineCacheCI.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
			pipelineCacheCI.initialDataSize = cacheSize;
			pipelineCacheCI.pInitialData = cacheData.data();

			vkCreatePipelineCache(m_Device, &pipelineCacheCI, VK_NULL_HANDLE, &m_PipelineCache);
		}
		else
		{
			VkPipelineCacheCreateInfo pipelineCacheCI = {};
			pipelineCacheCI.sType					  = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
			vkCreatePipelineCache(m_Device, &pipelineCacheCI, VK_NULL_HANDLE, &m_PipelineCache);
		}

	}

#ifdef USE_VMA_ALLOCATOR
	VmaPool VulkanDevice::GetOrCreateSmallAllocPool(uint32_t p_MemTypeIndex)
	{
		if (m_SmallAllocPools.contains(p_MemTypeIndex))
			return m_SmallAllocPools[p_MemTypeIndex];

		YM_CORE_INFO(VULKAN_PREFIX "Creating VMA small objects pool for memory type index {0}", p_MemTypeIndex)

		VmaPoolCreateInfo pci		= {};
		pci.memoryTypeIndex			= p_MemTypeIndex;
		pci.flags					= VMA_POOL_CREATE_IGNORE_BUFFER_IMAGE_GRANULARITY_BIT;
		pci.blockSize				= 0;
		pci.minBlockCount			= 0;
		pci.maxBlockCount			= SIZE_MAX;
		pci.priority				= 0.5f;
		pci.minAllocationAlignment  = 0;
		pci.pMemoryAllocateNext		= nullptr;
		VmaPool pool				= VK_NULL_HANDLE;

		auto res					= vmaCreatePool(m_Allocator, &pci, &pool);
		YM_CORE_VERIFY(res == VK_SUCCESS)

		m_SmallAllocPools[p_MemTypeIndex] = pool;

		return pool;
	}
#endif

}
