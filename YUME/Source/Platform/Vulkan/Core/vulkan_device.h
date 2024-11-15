#pragma once

#include "YUME/Core/base.h"
#include "YUME/Core/reference.h"
#include "vulkan_base.h"
#include "YUME/Core/singleton.h"
#include "vulkan_commandpool.h"
#include "vulkan_descriptor_pool.h"


#include <vulkan/vulkan.h>



namespace YUME
{
	struct QueueFamilyIndices
	{
		int Graphics = -1;
		int Compute = -1;
		int Transfer = -1;
	};

	struct PhysicalDeviceInfo
	{
		std::string GetVendorName() const;
		std::string DecodeVersion(const uint32_t p_Version) const;

		uint32_t Memory;
		uint32_t VendorID;
		std::string Driver;
		std::string APIVersion;
		std::string Vendor;
		std::string Name;
		VkPhysicalDeviceType Type;
	};

	struct PhysicalDevice
	{
		VkPhysicalDevice Handle;
		VkPhysicalDeviceProperties Properties;
		VkPhysicalDeviceFeatures Features;
		std::vector<VkQueueFamilyProperties> FamilyProperties;
		VkPhysicalDeviceMemoryProperties MemoryProperties;
		std::vector<VkExtensionProperties> SupportedExtensions;
		std::vector<VkDeviceQueueCreateInfo> QueueCreateInfos;

		PhysicalDeviceInfo Info;
		QueueFamilyIndices Indices;
		bool SupportCompute = false;
	};



	class VulkanPhysicalDevice
	{
		public:
			VulkanPhysicalDevice();
			~VulkanPhysicalDevice() = default;

			bool IsExtensionSupported(const char* p_Extension);

			PhysicalDevice& Selected() { return m_PhysicalDevices[m_SelectedIndex]; }
			VkPhysicalDevice& Handle() { return m_PhysicalDevices[m_SelectedIndex].Handle; }

			uint32_t FindMemoryType(uint32_t p_TypeFilter, VkMemoryPropertyFlags p_Properties) const;

		private:
			PhysicalDeviceInfo GetInfo(VkPhysicalDevice p_Device) const;

		private:
			uint32_t m_GPUCount = 0;
			std::vector<PhysicalDevice> m_PhysicalDevices;
			int m_SelectedIndex = 0;

			friend class VulkanDevice;
	};

	class VulkanDevice : public ThreadSafeSingleton<VulkanDevice>
	{
		friend class ThreadSafeSingleton<VulkanDevice>;

		public:
			VulkanDevice() = default;
			~VulkanDevice();

			void Init();

			VkPhysicalDevice& GetPhysicalDevice() { return m_PhysicalDevice->Handle(); }
			PhysicalDevice& GetPhysicalDeviceStruct() { return m_PhysicalDevice->Selected(); }
			const VkPhysicalDeviceFeatures& GetFeatures() const { return m_PhysicalDevice->Selected().Features; }
			const QueueFamilyIndices& GetQueueFamilyIndices() const { return m_PhysicalDevice->Selected().Indices; }
			bool SupportCompute() const { return m_PhysicalDevice->Selected().SupportCompute; }

			uint32_t FindMemoryType(uint32_t p_TypeFilter, VkMemoryPropertyFlags p_Properties) const { return m_PhysicalDevice->FindMemoryType(p_TypeFilter, p_Properties); }

			VkDevice& GetDevice() { return m_Device; }

			VkPipelineCache GetPipelineCache() const { return m_PipelineCache; }

			VkQueue& GetGraphicQueue() { return m_GraphicQueue; }
			VkQueue& GetTransferQueue() { return m_TransferQueue; }
			VkQueue& GetComputeQueue() { return m_ComputeQueue; }

			VkCommandPool GetCommandPool() { return m_CommandPool->GetHandle(); }
			VkDescriptorPool GetDescriptorPool() { return m_DescriptorPool->Get(); }
			void ResetDescriptorPool() { m_DescriptorPool->Reset(); }
			void ResetCommandPool() { m_CommandPool->Reset(); }

		#ifdef USE_VMA_ALLOCATOR
			VmaAllocator GetAllocator() const
			{
				return m_Allocator;
			}

			VmaPool GetOrCreateSmallAllocPool(uint32_t p_MemTypeIndex);
		#endif


		private:
			Unique<VulkanPhysicalDevice> m_PhysicalDevice;

			VkDevice m_Device;

			VkPipelineCache m_PipelineCache;
			std::filesystem::path m_PipelineCacheDir = "assets/cache/renderer";
			std::filesystem::path m_PipelineCachePath = m_PipelineCacheDir / "vulkan_pipeline.cache";

			VkQueue m_GraphicQueue;
			VkQueue m_TransferQueue;
			VkQueue m_ComputeQueue;

			Ref<VulkanCommandPool> m_CommandPool;
			Ref<VulkanDescriptorPool> m_DescriptorPool;

		#ifdef USE_VMA_ALLOCATOR
			VmaAllocator m_Allocator = VK_NULL_HANDLE;
			std::unordered_map<uint32_t, VmaPool> m_SmallAllocPools;
		#endif
			
		
	};
}