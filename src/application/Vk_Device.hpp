#pragma once

#include <vector>
#include <type_traits>
#include <map>
#include <unordered_map>
#include <functional>
#include <set>
#include <queue>

#include "../Defines.h"
#include "Vk_DeviceLib.hpp"
#include "Vk_Surface.hpp"
#include "Vk_Instance.hpp"

namespace VK4 {
	class  Vk_Device {
	public:
		enum class CommandCapabilities {
			Render,
			RuntimeCopy,
			Initialization
		};

		Vk_Device(std::string deviceName, Vk_DevicePreference devicePreference = Vk_DevicePreference::USE_ANY_GPU)
			:
			bridge(Vk_DeviceLib::Bridge(0)),
			_viewer(0),
			_device(nullptr),
			_graphicsQueues({}),
			_presentationQueues({}),
			_renderingCommandPool(nullptr),
			_copyCommandPool(nullptr),
			_initializationCommandPool(nullptr),
			_swapchainSupportDetails({}),
			_swapchainSupportDetailsUpToDate(false),
			_multiImageBuffering(true),
			_physicalDevices({}),
			_activePhysicalDevice(nullptr),
			_physicalDeviceIndex(0),
			_instance(nullptr),
			_gpuMemoryConfig({}),
			_gpuHeapConfig({}),
			_devicePreference(devicePreference)
		{
			_instance = std::make_unique<Vk_Instance>(deviceName);

			// put this after the _instance generation so that the destruction sequence output is correct
			// (instance after device...)

			Vk_Logger::Log(typeid(this), GlobalCasters::castConstructorTitle("Create Device"));
			vk_invalidateSwapchainSupport();

			auto surface = Vk_Surface(_instance.get(), "temp", 1, 1, false, false);
			configDeviceForSurface(&surface);
			vk_invalidateSwapchainSupport(); // we still need to ask for support because we just created a 1x1 surface here
			_setGpuMemoryConfig();
		}

		~Vk_Device() {
			Vk_Logger::Log(typeid(this), GlobalCasters::castDestructorTitle("Destroy Device"));
			Vk_ThreadSafe::Vk_ThreadSafe_DeviceWaitIdle(_device);

			vkDestroyCommandPool(_device, _copyCommandPool, nullptr);
			vkDestroyCommandPool(_device, _renderingCommandPool, nullptr);
			vkDestroyCommandPool(_device, _initializationCommandPool, nullptr);
			vkDestroyDevice(_device, nullptr);
			_instance.reset();
		}

		VkPhysicalDevice vk_pDev() const {
			return _activePhysicalDevice->physicalDevice;
		}

		const std::vector<Vk_DeviceLib::PhysicalDevice>& vk_allPhysicalDevices() const {
			return _physicalDevices;
		}

		VkSampleCountFlagBits vk_maxUsableSampleCount() const {
			return _activePhysicalDevice->maxUsableSampleCount;
		}

		const VkDevice vk_lDev() const {
			return _device;
		}

		const VkQueue vk_graphicsQueue() const {
			return _graphicsQueues[0];
		}

		const VkQueue vk_presentationQueue() const {
			return _presentationQueues[0];
		}

		const VkQueue vk_transferQueue() const {
			return _transferQueues[0];
		}

		const VkCommandPool vk_renderingCommandPool() const {
			return _renderingCommandPool;
		}

		const std::uint64_t vk_queryInstalledMemory(VkMemoryPropertyFlags flags) const {
			const auto conf = _gpuMemoryConfig.at(_activePhysicalDevice).find(flags);
			if (conf == _gpuMemoryConfig.at(_activePhysicalDevice).end()) {
				return 0;
			}

			return _gpuHeapConfig.at(_activePhysicalDevice).at(conf->second.heapIndex).heapSize;
		}

		const std::uint64_t vk_queryMemoryBudget(VkMemoryPropertyFlags flags) const {
			auto conf = _gpuMemoryConfig.at(_activePhysicalDevice).find(flags);
			if (conf == _gpuMemoryConfig.at(_activePhysicalDevice).end()) {
				return 0;
			}

			return _gpuHeapConfig.at(_activePhysicalDevice).at(conf->second.heapIndex).heapBudget;
		}

		const std::uint64_t vk_queryMemoryUsage(VkMemoryPropertyFlags flags) const {
			auto conf = _gpuMemoryConfig.at(_activePhysicalDevice).find(flags);
			if (conf == _gpuMemoryConfig.at(_activePhysicalDevice).end()) {
				return 0;
			}

			return _gpuHeapConfig.at(_activePhysicalDevice).at(conf->second.heapIndex).heapUsage;
		}

		void printActiveDeviceMemoryProperties(std::ostream& stream = std::cout) {
			Vk_DeviceLib::printActiveDeviceMemoryProperties(_physicalDevices, _gpuHeapConfig, stream);
		}

		void vk_invalidateSwapchainSupport() {
			_swapchainSupportDetailsUpToDate = false;
		}

		const SwapchainSupportDetails& vk_swapchainSupportActiveDevice(const Vk_Surface* surface) {
			return vk_swapchainSupport(_activePhysicalDevice, surface);
		}

		bool vk_testAndUpdateNFramesInFlight(uint32_t swapchainImageCount){
			if(_swapchainSupportDetailsUpToDate){
				if(swapchainImageCount != _swapchainSupportDetails.nFramesInFlight){
					Vk_Logger::Log(typeid(this), "nFramesInFlight changed to " + std::to_string(swapchainImageCount) + " from " + std::to_string(_swapchainSupportDetails.nFramesInFlight) + " due to swapchain initialization requirements!");
					_swapchainSupportDetails.nFramesInFlight = swapchainImageCount;
					
					bridge = Vk_DeviceLib::Bridge(_swapchainSupportDetails.nFramesInFlight);
				}
			}
			return _swapchainSupportDetailsUpToDate;
		}

		const Vk_DeviceLib::QueueFamilyIndex* vk_queueFamilyIndices() const {
			return &_activePhysicalDevice->queueFamilyIndices;
		}

		const uint32_t vk_graphicsFamilyIndex() const {
			return static_cast<uint32_t>(_activePhysicalDevice->queueFamilyIndices.graphicsFamilyIndex);
		}

		const uint32_t vk_presentFamilyIndex() const {
			return static_cast<uint32_t>(_activePhysicalDevice->queueFamilyIndices.presentFamilyIndex);
		}

		const VkSampleCountFlagBits vk_sampleCount() const {
			return _activePhysicalDevice->maxUsableSampleCount;
		}

		void vk_submitWork(VkCommandBuffer cmdBuffer) {
			auto lock = AcquireGlobalWriteLock("vk_device[vk_submitWork]");
			Vk_DeviceLib::submitWork(_device, cmdBuffer, _graphicsQueues[0]);
		}

		void vk_copyDeviceBufferToVector(void* dstPtr, VkDeviceMemory deviceBufferMemory, VkDeviceSize size) {
			auto lock = AcquireGlobalWriteLock("vk_device[vk_copyDeviceBufferToVector]");
			Vk_DeviceLib::copyDeviceBufferToVector(_device, dstPtr, deviceBufferMemory, size);
		}

		void vk_createCopyImgToBufferCommand(
			VkCommandBuffer& copyCmdBuffer,
			VkImage srcImage,
			VkBuffer dstBuffer,
			VkExtent3D imageExtent
		) {
			Vk_DeviceLib::createCopyImgToBufferCommand(
				_device,
				selectCommandPool(CommandCapabilities::RuntimeCopy),
				copyCmdBuffer, srcImage, dstBuffer, imageExtent
			);
		}

		void vk_createSwapchainResource(
			const VkImageTiling tiling,
			const VkImageUsageFlags imageUsageFlags,
			const VkMemoryPropertyFlags usageFlags,
			const VkImageAspectFlags imageAspectFlags,
			const VkExtent2D& extent2D,
			const VkFormat& colorFormat,
			const uint32_t miplevels,
			const VkImageLayout oldLayout,
			const VkImageLayout newLayout,
			VkImage& swapchainImage,
			VkDeviceMemory& swapchainImageMemory,
			VkImageView& swapchainImageView
		) {
			Vk_DeviceLib::createSwapchainResource(
				_device, _activePhysicalDevice,
				tiling, imageUsageFlags, usageFlags, imageAspectFlags, 
				extent2D, colorFormat, miplevels, oldLayout, newLayout,
				selectCommandPool(CommandCapabilities::Initialization),
				selectQueue(CommandCapabilities::Initialization),
				swapchainImage, swapchainImageMemory, swapchainImageView
			);
		}

		VkResult vk_createBuffer(
			VkBufferUsageFlags usageFlags,
			VkMemoryPropertyFlags memoryPropertyFlags,
			VkBuffer& buffer,
			VkDeviceMemory& memory,
			VkDeviceSize size,
			void* data = nullptr
		) {
			return Vk_DeviceLib::createBuffer(
				_activePhysicalDevice, _device,
				usageFlags, memoryPropertyFlags,
				buffer, memory, size, data
			);
		}

		void vk_copyBuffer(
			CommandCapabilities command,
			VkBuffer& srcBuffer,
			VkBuffer& dstBuffer,
			VkDeviceSize size,
			VkDeviceSize srcOffset = 0,
			VkDeviceSize dstOffset = 0,
			const std::string& associatedObject = ""
		) {
			Vk_DeviceLib::copyBuffer(
				_device, selectCommandPool(command), selectQueue(command),
				srcBuffer, srcOffset, dstBuffer, dstOffset,
				size, associatedObject
			);
		}

		void vk_copyBuffer(
			CommandCapabilities command,
			VkBuffer& srcBuffer,
			VkDeviceSize srcOffset,
			VkBuffer& dstBuffer,
			VkDeviceSize dstOffset,
			VkDeviceSize size,
			const std::string& associatedObject = ""
		) {
			Vk_DeviceLib::copyBuffer(
				_device, selectCommandPool(command), selectQueue(command),
				srcBuffer, srcOffset, dstBuffer, dstOffset,
				size, associatedObject
			);
		}

		inline VkImageView vk_createImageView(
			VkImage image,
			VkFormat format,
			VkImageAspectFlags aspectFlags,
			uint32_t miplevels
		) {
			return Vk_DeviceLib::createImageView(_device, image, format, aspectFlags, miplevels);
		}

		Vk_DeviceLib::SingleTimeCommand vk_beginSingleTimeCommands(CommandCapabilities command) {
			return Vk_DeviceLib::beginSingleTimeCommands(_device, selectCommandPool(command), selectQueue(command));
		}

		void vk_endSingleTimeCommands(const Vk_DeviceLib::SingleTimeCommand& singleTimeCommand) {
			Vk_DeviceLib::endSingleTimeCommands(_device, singleTimeCommand);
		}

		Vk_Instance* vk_instance() {
			return _instance.get();
		}

		bool vk_registerViewer(uint64_t viewer){
			if(_viewer != 0 && _viewer != viewer) return false;
			_viewer = viewer;
			return true;
		}

		void vk_unregisterViewer(){
			_viewer = 0;
		}
		
		Vk_DeviceLib::Bridge bridge;

	private:
		uint64_t _viewer;
		VkDevice _device;
		std::vector<VkQueue> _graphicsQueues;
		std::vector<VkQueue> _presentationQueues;
		std::vector<VkQueue> _transferQueues;

		VkCommandPool _renderingCommandPool;
		VkCommandPool _copyCommandPool;
		VkCommandPool _initializationCommandPool;

		SwapchainSupportDetails _swapchainSupportDetails;
		bool _swapchainSupportDetailsUpToDate;
		bool _multiImageBuffering;

		std::vector<Vk_DeviceLib::PhysicalDevice> _physicalDevices;
		Vk_DeviceLib::PhysicalDevice* _activePhysicalDevice;
		int _physicalDeviceIndex;
		std::unique_ptr<Vk_Instance> _instance;
		Vk_DeviceLib::TGpuMemoryConfig _gpuMemoryConfig;
		Vk_DeviceLib::TGpuHeapConfig _gpuHeapConfig;

		Vk_DevicePreference _devicePreference; 

		void configDeviceForSurface(Vk_Surface* surface) {
			bool res = _setPhysicalDevice(surface);
			if (!res) Vk_Logger::RuntimeError(typeid(this), "Suitable physical device (GPU) not found");

			if (_physicalDeviceIndex < 0) {
				Vk_Logger::RuntimeError(typeid(this), "No suitable physical device assigned!");
			}

			std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
			std::set<int> uniqueQueueFamilies = {
				_activePhysicalDevice->queueFamilyIndices.graphicsFamilyIndex,
				_activePhysicalDevice->queueFamilyIndices.presentFamilyIndex,
				_activePhysicalDevice->queueFamilyIndices.transferFamilyIndex,
			};

			// create the two queues (graphics and presentation). For now, we stick
			// with one each, but we may try to extend this in the future
			float queuePriority = 1.0f;
			for (int queueFamily : uniqueQueueFamilies) {
				VkDeviceQueueCreateInfo queueCreateInfo = { VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
				queueCreateInfo.queueFamilyIndex = queueFamily;
				queueCreateInfo.queueCount = 1;
				queueCreateInfo.pQueuePriorities = &queuePriority;
				queueCreateInfos.push_back(queueCreateInfo);
			}	

			VkDeviceCreateInfo deviceCreateInfo = { VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
			deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
			deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
			deviceCreateInfo.pEnabledFeatures = &_activePhysicalDevice->deviceFeatures;

			// read the device extension names into a vector and pass it to the device creation struct
			// perform all required support tests here
			if (!_activePhysicalDevice->supportsSwapchainExtension()) {
				Vk_Logger::RuntimeError(typeid(this), "Swapchain is not suported by current physical device (GPU)");
			}

			std::vector<const char*> devExtensions = _activePhysicalDevice->getMinimumRequiredExtensions();
			deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(devExtensions.size());
			deviceCreateInfo.ppEnabledExtensionNames = devExtensions.data();

			Vk_CheckVkResult(typeid(this), 
				vkCreateDevice(_activePhysicalDevice->physicalDevice, &deviceCreateInfo, nullptr, &_device),
				"Failed to create logical device"
			);

			_graphicsQueues.resize(1);
			_presentationQueues.resize(1);
			_transferQueues.resize(1);
			vkGetDeviceQueue(_device, _activePhysicalDevice->queueFamilyIndices.graphicsFamilyIndex, 0, &_graphicsQueues[0]);
			vkGetDeviceQueue(_device, _activePhysicalDevice->queueFamilyIndices.presentFamilyIndex, 0, &_presentationQueues[0]);
			vkGetDeviceQueue(_device, _activePhysicalDevice->queueFamilyIndices.transferFamilyIndex, 0, &_transferQueues[0]);

			// create command pool for the device graphics queue
			VkCommandPoolCreateInfo cmdPoolInfo = {};
			cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			cmdPoolInfo.queueFamilyIndex = _activePhysicalDevice->queueFamilyIndices.graphicsFamilyIndex;
			cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
			Vk_CheckVkResult(typeid(this), 
				vkCreateCommandPool(_device, &cmdPoolInfo, nullptr, &_renderingCommandPool),
				"Unable to create command pool"
			);

			// create command pool for the device transfer queue
			cmdPoolInfo = {};
			cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			cmdPoolInfo.queueFamilyIndex = _activePhysicalDevice->queueFamilyIndices.transferFamilyIndex;
			cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
			Vk_CheckVkResult(typeid(this), 
				vkCreateCommandPool(_device, &cmdPoolInfo, nullptr, &_copyCommandPool),
				"Unable to create command pool"
			);

			// create command pool for the device initialization queue
			cmdPoolInfo = {};
			cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			cmdPoolInfo.queueFamilyIndex = _activePhysicalDevice->queueFamilyIndices.graphicsFamilyIndex;
			cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
			Vk_CheckVkResult(typeid(this), 
				vkCreateCommandPool(_device, &cmdPoolInfo, nullptr, &_initializationCommandPool),
				"Unable to create command pool"
			);
		}

		inline VkCommandPool selectCommandPool(CommandCapabilities command) {
			switch (command) {
			case CommandCapabilities::Render:
				return _renderingCommandPool;
			case CommandCapabilities::RuntimeCopy:
				return _copyCommandPool;
			case CommandCapabilities::Initialization:
				return _initializationCommandPool;
			default:
				Vk_Logger::RuntimeError(typeid(this), "Unsuported command capability");
			}
			return nullptr;
		}

		inline VkQueue selectQueue(CommandCapabilities command) {
			switch (command) {
			case CommandCapabilities::Render:
				return _graphicsQueues[0];
			case CommandCapabilities::RuntimeCopy:
				return _transferQueues[0];
			case CommandCapabilities::Initialization:
				return _presentationQueues[0];
			default:
				Vk_Logger::RuntimeError(typeid(this), "Unsuported command capability");
			}
			return nullptr;
		}

		const SwapchainSupportDetails& vk_swapchainSupport(Vk_DeviceLib::PhysicalDevice* pDev, const Vk_Surface* surface) {
			// if we added a new camera, we need to reaquire all subsequent details because they may have moved
			// such that the address is no longer valid
			// int camId = 0;
			// if (camId >= 0) {
			// 	if (_swapchainSupportDetailsUpToDate.find(camId) == _swapchainSupportDetailsUpToDate.end()) {
			// 		_swapchainSupportDetailsUpToDate[camId] = false;
			// 	}
			// 	else if (_swapchainSupportDetailsUpToDate.at(camId))
			// 		return swapchainSupportDetails.at(camId);
			// }

			if(_swapchainSupportDetailsUpToDate){
				return _swapchainSupportDetails;
			}

			_swapchainSupportDetails = Vk_DeviceLib::swapchainSupport(pDev, surface, _multiImageBuffering, bridge);

			if(surface != nullptr){
				// only set this to updated state if the query was made including a surface
				// if this wasn't the case, the query will not retrieve the completed set of infos
				// but also will not crash the program
				_swapchainSupportDetailsUpToDate = true;
			}
			return _swapchainSupportDetails;
		}

		// choose the best graphics supporting device out of all devices
		bool _setPhysicalDevice(Vk_Surface* surface)
		{
			return Vk_DeviceLib::setPhysicalDevice(_instance.get(), surface, _physicalDevices, _multiImageBuffering, _devicePreference, bridge, _physicalDeviceIndex, _activePhysicalDevice);
		}

		void _setGpuMemoryConfig() {
			Vk_DeviceLib::setGpuMemoryConfig(_physicalDevices, _gpuHeapConfig, _gpuMemoryConfig);
		}
	};
}
