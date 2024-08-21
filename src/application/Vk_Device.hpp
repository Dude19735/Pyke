#pragma once

#include <vector>
#include <type_traits>
#include <map>
#include <unordered_map>
#include <functional>
#include <set>
#include <queue>


#include "../Defines.h"
// #include "../Vk_Logger.hpp"
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
private:

		



	public:

		friend class Vk_Viewer;

		Vk_Device(std::string deviceName, Vk_DevicePreference devicePreference = Vk_DevicePreference::USE_ANY_GPU)
			:
			bridge(Vk_DeviceLib::Bridge{.currentFrame=0}),
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
			// glfwInit();
			_instance = std::make_unique<Vk_Instance>(deviceName);

			// put this after the _instance generation so that the destruction sequence output is correct
			// (instance after device...)

			Vk_Logger::Log(typeid(this), GlobalCasters::castConstructorTitle("Create Device"));

			// glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
			// GLFWwindow* window = glfwCreateWindow(initWidth, initHeight, "temp", nullptr, nullptr);
			// Vk_Surface surface(_instance.get(), "Temporal Surface for Device Capabilities Query");

			setGpuMemoryConfig();

			vk_invalidateSwapchainSupport();

			auto surface = Vk_Surface(_instance.get(), "temp", 1, 1, false, false);
			vk_configDeviceForSurface(&surface);
			vk_invalidateSwapchainSupport(); // we still need to ask for support because we just created a 1x1 surface here
		}
		~Vk_Device() {
			Vk_Logger::Log(typeid(this), GlobalCasters::castDestructorTitle("Destroy Device"));
			Vk_ThreadSafe::Vk_ThreadSafe_DeviceWaitIdle(_device);
			// vk_cleanSingleTimeCommands(true);

			vkDestroyCommandPool(_device, _copyCommandPool, nullptr);
			vkDestroyCommandPool(_device, _renderingCommandPool, nullptr);
			vkDestroyCommandPool(_device, _initializationCommandPool, nullptr);
			vkDestroyDevice(_device, nullptr);
			_instance.reset();
		}

		VkPhysicalDevice vk_pDev() {
			return _activePhysicalDevice->physicalDevice;
		}

		VkSampleCountFlagBits vk_maxUsableSampleCount() {
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

		//SwapchainSupportDetails* vk_updateSwapchainSupportDetails() {
		//	_swapchainSupportDetails = vk_swapchainSupportActiveDevice(_surface);
		//}

		//inline VkExtent2D chooseSwapExtent(
		//	const VkSurfaceCapabilitiesKHR& capabilities,
		//	const int width,
		//	const int height
		//) {
		//	//_swapchainSupportDetails
		//	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
		//		// if the maximal extend is not the maximal value of an uint32 then just take the current size in here
		//		return capabilities.currentExtent;
		//	}
		//	else {
		//		// otherwise make it as large as can possibly fit on the screen
		//		// get the size of the window from glfw

		//		VkExtent2D actualExtent = {
		//			static_cast<uint32_t>(width),
		//			static_cast<uint32_t>(height)
		//		};

		//		actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
		//		actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));
		//		return actualExtent;
		//	}
		//}

		void printActiveDeviceMemoryProperties(std::ostream& stream = std::cout) {
			Vk_DeviceLib::printActiveDeviceMemoryProperties(_physicalDevices, _gpuHeapConfig, stream);
			// updateGpuHeapUsageStats();
			// tabulate::Table table;
			// table.add_row({"GPU", "Heap Size", "Usage", "Heap Config Possibilities"});
			// table.format().font_style({ tabulate::FontStyle::bold })
			// 	.border_top(" ").border_bottom(" ")
			// 	.border_left(" ").border_right(" ")
			// 	.corner(" ");
			// table[0].format()
			// 	.padding_top(1)
			// 	.padding_bottom(1)
			// 	.font_align(tabulate::FontAlign::center)
			// 	.font_style({ tabulate::FontStyle::underline })
			// 	.font_background_color(tabulate::Color::red);

			// for (auto& item : _gpuHeapConfig) {
			// 	for (size_t i = 0; i < item.second.size(); ++i) {
			// 		auto h = item.second.at(i); // _gpuHeapConfig[_activePhysicalDevice].at(i);
			// 		std::string s_kb = rightCrop(h.heapSizeKb) + "[Kb]";
			// 		std::string s_mb = rightCrop(h.heapSizeMb) + "[MB]";
			// 		std::string s_gb = rightCrop(h.heapSizeGb) + "[GB]";

			// 		std::string usage_kb = rightCrop(h.heapUsageKb) + " / " + rightCrop(h.heapBudgetKb) + "[Kb]";
			// 		std::string usage_mb = rightCrop(h.heapUsageMb) + " / " + rightCrop(h.heapBudgetMb) + "[MB]";
			// 		std::string usage_gb = rightCrop(h.heapUsageGb) + " / " + rightCrop(h.heapBudgetGb) + "[GB]";

			// 		std::string props = "";
			// 		for (auto conf : h.heapConfigs) {
			// 			props.append(conf->flagsStr);
			// 			props.append("\n");
			// 		}
			// 		//std::cout << std::string(item.first->deviceProperties.deviceName) << std::endl;
			// 		table.add_row({
			// 			std::string(item.first->deviceProperties.deviceName),
			// 			s_kb + "\n" + s_mb + "\n" + s_gb,
			// 			usage_kb + "\n" + usage_mb + "\n" + usage_gb,
			// 			props });
			// 	}
			// }

			// table.column(0).format().font_color(tabulate::Color::red);
			// table.column(1).format().font_color(tabulate::Color::blue);
			// table.column(2).format().font_color(tabulate::Color::cyan);
			// table.column(3).format().font_color(tabulate::Color::yellow);

			// table[0][0].format().font_background_color(tabulate::Color::red).font_color(tabulate::Color::white);
			// table[0][1].format().font_background_color(tabulate::Color::blue).font_color(tabulate::Color::white);
			// table[0][2].format().font_background_color(tabulate::Color::cyan).font_color(tabulate::Color::blue);
			// table[0][3].format().font_background_color(tabulate::Color::yellow).font_color(tabulate::Color::blue);

			// stream << table << std::endl;
		}

		void vk_configDeviceForSurface(Vk_Surface* surface) {
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

			VK_CHECK(
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
			VK_CHECK(
				vkCreateCommandPool(_device, &cmdPoolInfo, nullptr, &_renderingCommandPool),
				"Unable to create command pool"
			);

			// create command pool for the device transfer queue
			cmdPoolInfo = {};
			cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			cmdPoolInfo.queueFamilyIndex = _activePhysicalDevice->queueFamilyIndices.transferFamilyIndex;
			cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
			VK_CHECK(
				vkCreateCommandPool(_device, &cmdPoolInfo, nullptr, &_copyCommandPool),
				"Unable to create command pool"
			);

			// create command pool for the device initialization queue
			cmdPoolInfo = {};
			cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			cmdPoolInfo.queueFamilyIndex = _activePhysicalDevice->queueFamilyIndices.graphicsFamilyIndex;
			cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
			VK_CHECK(
				vkCreateCommandPool(_device, &cmdPoolInfo, nullptr, &_initializationCommandPool),
				"Unable to create command pool"
			);
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
					
					bridge.updates.clear();
					for(uint32_t i=0; i<_swapchainSupportDetails.nFramesInFlight; ++i){
						auto q = std::queue<std::function<void()>>();
						bridge.updates[i] = q;
					}
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
			auto lock = AcquireGlobalLock("vk_device[vk_submitWork]");
			Vk_DeviceLib::submitWork(_device, cmdBuffer, _graphicsQueues[0]);

			// VkSubmitInfo submitInfo{};
			// submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			// submitInfo.commandBufferCount = 1;
			// submitInfo.pCommandBuffers = &cmdBuffer;

			// VkFenceCreateInfo fenceCreateInfo{};
			// fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
			// fenceCreateInfo.flags = 0;
			// VkFence fence;

			// VK_CHECK(vkCreateFence(_device, &fenceCreateInfo, nullptr, &fence), "Unable to create fence");

			// VkResult res = Vk_ThreadSafe::Vk_ThreadSafe_QueueSubmit(_graphicsQueues[0], 1, &submitInfo, fence);
			// VK_CHECK(res, "Unable to submit fence to queue");

			// VK_CHECK(vkWaitForFences(_device, 1, &fence, VK_TRUE, UINT64_MAX), "Unable to wait for fences");
			// vkDestroyFence(_device, fence, nullptr);
		}

		void vk_copyDeviceBufferToVector(void* dstPtr, VkDeviceMemory deviceBufferMemory, VkDeviceSize size) {
			// const char* data;
			// Map image memory so we can start copying from it
			auto lock = AcquireGlobalLock("vk_device[vk_copyDeviceBufferToVector]");
			Vk_DeviceLib::copyDeviceBufferToVector(_device, dstPtr, deviceBufferMemory, size);
			// vkMapMemory(_device, deviceBufferMemory, 0, size, 0, (void**)&data);

			// memcpy(dstPtr, (void*)data, (size_t)size);

			// // Clean up resources
			// vkUnmapMemory(_device, deviceBufferMemory);
			// Vk_ThreadSafe::Vk_ThreadSafe_QueueWaitIdle(_graphicsQueues[0]);
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
			// // Do the actual blit from the offscreen image to our host visible destination image
			// VkCommandBufferAllocateInfo cmdBufAllocateInfo{};
			// cmdBufAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			// cmdBufAllocateInfo.commandPool = selectCommandPool(CommandCapabilities::RuntimeCopy);
			// cmdBufAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			// cmdBufAllocateInfo.commandBufferCount = 1;

			// VK_CHECK(vkAllocateCommandBuffers(_device, &cmdBufAllocateInfo, &copyCmdBuffer), "Unable to command buffers");

			// VkCommandBufferBeginInfo cmdBufferBeginInfo{};
			// cmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			// VK_CHECK(vkBeginCommandBuffer(copyCmdBuffer, &cmdBufferBeginInfo), "Unable to begin command buffer recording");

			// insertImageMemoryBarrier(
			// 	copyCmdBuffer,
			// 	srcImage,
			// 	0,
			// 	VK_ACCESS_TRANSFER_WRITE_BIT,
			// 	VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
			// 	VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			// 	VK_PIPELINE_STAGE_TRANSFER_BIT,
			// 	VK_PIPELINE_STAGE_TRANSFER_BIT,
			// 	VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 }
			// );

			// createCopyToBufferCommand(
			// 	copyCmdBuffer,
			// 	srcImage,
			// 	dstBuffer,
			// 	VK_IMAGE_ASPECT_COLOR_BIT,
			// 	imageExtent
			// );

			// insertImageMemoryBarrier(
			// 	copyCmdBuffer,
			// 	srcImage,
			// 	0,
			// 	VK_ACCESS_TRANSFER_WRITE_BIT,
			// 	VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			// 	VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
			// 	VK_PIPELINE_STAGE_TRANSFER_BIT,
			// 	VK_PIPELINE_STAGE_TRANSFER_BIT,
			// 	VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 }
			// );

			// VK_CHECK(vkEndCommandBuffer(copyCmdBuffer), "Unable to end command buffer recording");// Transition destination image to transfer destination layout
		}

		inline void vk_createSwapchainResource(
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
			// //!create image
			// vk_createImage(
			// 	extent2D.width,
			// 	extent2D.height,
			// 	miplevels,
			// 	_activePhysicalDevice->maxUsableSampleCount,
			// 	colorFormat,
			// 	tiling,
			// 	imageUsageFlags,
			// 	usageFlags,
			// 	swapchainImage,
			// 	swapchainImageMemory
			// );

			// //! create image view
			// swapchainImageView = vk_createImageView(
			// 	swapchainImage,
			// 	colorFormat,
			// 	imageAspectFlags,
			// 	miplevels
			// );

			// //! transition image view to some faster memory type
			// vk_transitionImageLayout(
			// 	CommandCapabilities::Initialization,
			// 	swapchainImage,
			// 	colorFormat,
			// 	oldLayout,
			// 	newLayout,
			// 	miplevels
			// );
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
			// // Create the buffer handle
			// VkBufferCreateInfo bufferCreateInfo{};
			// bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			// bufferCreateInfo.usage = usageFlags;
			// bufferCreateInfo.size = size;
			// bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			// VK_CHECK(vkCreateBuffer(_device, &bufferCreateInfo, nullptr, &buffer), "Unable to create buffer");

			// // Create the memory backing up the buffer handle
			// VkMemoryRequirements memReqs;
			// VkMemoryAllocateInfo memAllocInfo{};
			// memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			// vkGetBufferMemoryRequirements(_device, buffer, &memReqs);
			// memAllocInfo.allocationSize = memReqs.size;

			// memAllocInfo.memoryTypeIndex = getMemoryTypeIndex(memReqs.memoryTypeBits, memoryPropertyFlags);
			// VkResult res = vkAllocateMemory(_device, &memAllocInfo, nullptr, &memory);
			// if (res != VK_SUCCESS) {
			// 	if (res == VK_ERROR_OUT_OF_DEVICE_MEMORY) {
			// 		throw OutOfDeviceMemoryException();
			// 	}
			// 	else {
			// 		Vk_Logger::RuntimeError(typeid(this), "failed to allocate buffer memory!");
			// 	}
			// }

			// if (data != nullptr) {
			// 	void* mapped;
			// 	VK_CHECK(vkMapMemory(_device, memory, 0, size, 0, &mapped), "Unable to map memory for buffer creation");
			// 	memcpy(mapped, data, size);
			// 	vkUnmapMemory(_device, memory);
			// }

			// VK_CHECK(vkBindBufferMemory(_device, buffer, memory, 0), "Unable to bind buffer memory to device");

			// return VK_SUCCESS;
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
			// if(size == 0){
			// 	Vk_Logger::Error(typeid(this), "Attempted to copy 0 bytes to device buffer {0}", associatedObject);
			// 	return;
			// }

			// SingleTimeCommand singleTimeCommand = vk_beginSingleTimeCommands(command);

			// VkBufferCopy copyRegion = {};
			// copyRegion.srcOffset = srcOffset; // optional
			// copyRegion.dstOffset = dstOffset; // optional
			// copyRegion.size = size;
			// vkCmdCopyBuffer(singleTimeCommand.commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

			// vk_endSingleTimeCommands(singleTimeCommand);
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

			// if(size == 0){
			// 	Vk_Logger::Error(typeid(this), "Attempted to copy 0 bytes to device buffer {0}", associatedObject);
			// 	return;
			// }

			// SingleTimeCommand singleTimeCommand = vk_beginSingleTimeCommands(command);

			// VkBufferCopy copyRegion = {};
			// copyRegion.srcOffset = srcOffset; // optional
			// copyRegion.dstOffset = dstOffset; // optional
			// copyRegion.size = size;
			// vkCmdCopyBuffer(singleTimeCommand.commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

			// vk_endSingleTimeCommands(singleTimeCommand);
		}

		inline VkImageView vk_createImageView(
			VkImage image,
			VkFormat format,
			VkImageAspectFlags aspectFlags,
			uint32_t miplevels
		) {
			return Vk_DeviceLib::createImageView(_device, image, format, aspectFlags, miplevels);

			// VkImageViewCreateInfo createInfo = {};
			// createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			// createInfo.image = image;
			// createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			// createInfo.format = format;

			// // use default mappings for every color channel
			// createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			// createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			// createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			// createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

			// createInfo.subresourceRange.aspectMask = aspectFlags;
			// createInfo.subresourceRange.baseMipLevel = 0;
			// createInfo.subresourceRange.levelCount = miplevels;
			// createInfo.subresourceRange.baseArrayLayer = 0;
			// createInfo.subresourceRange.layerCount = 1;

			// VkImageView imageView;
			// VK_CHECK(vkCreateImageView(_device, &createInfo, nullptr, &imageView), "Failed to create image view");

			// return imageView;
		}

		// inline void vk_createImage(
		// 	uint32_t width,
		// 	uint32_t height,
		// 	uint32_t mipLevels,
		// 	VkSampleCountFlagBits numSamples,
		// 	VkFormat format,
		// 	VkImageTiling tiling,
		// 	VkImageUsageFlags usage,
		// 	VkMemoryPropertyFlags properties,
		// 	VkImage& image,
		// 	VkDeviceMemory& imageMemory
		// ) {
		// 	// set up actual image object
		// 	VkImageCreateInfo imageInfo = {};
		// 	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		// 	imageInfo.imageType = VK_IMAGE_TYPE_2D;
		// 	imageInfo.extent.width = static_cast<uint32_t>(width);
		// 	imageInfo.extent.height = static_cast<uint32_t>(height);
		// 	imageInfo.extent.depth = 1;
		// 	imageInfo.mipLevels = mipLevels;
		// 	imageInfo.arrayLayers = 1;
		// 	imageInfo.format = format;
		// 	imageInfo.tiling = tiling;
		// 	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		// 	imageInfo.usage = usage;
		// 	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		// 	imageInfo.samples = numSamples;
		// 	imageInfo.flags = 0;

		// 	VK_CHECK(vkCreateImage(_device, &imageInfo, nullptr, &image), "Failed to create image");

		// 	VkMemoryRequirements memRequirements;
		// 	vkGetImageMemoryRequirements(_device, image, &memRequirements);

		// 	VkMemoryAllocateInfo allocInfo = {};
		// 	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		// 	allocInfo.allocationSize = memRequirements.size;
		// 	allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

		// 	if (vkAllocateMemory(_device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
		// 		Vk_Logger::RuntimeError(typeid(this), "failed to allocate image memory!");
		// 	}

		// 	// bind image to staging buffer memory
		// 	vkBindImageMemory(_device, image, imageMemory, 0);
		// }

		// void vk_cleanSingleTimeCommands(bool overrideCheck=false){
		// 	VkCommandPool pool = selectCommandPool(CommandCapabilities::RuntimeCopy);
		// 	int index = 0;
		// 	while(index < _singleTimeCommandBuffer.size()){
		// 		VkResult status = vkGetFenceStatus(_device, _singleTimeCommandBuffer[index].fence);
		// 		// std::cout << static_cast<uint64_t>(status) << std::endl;
		// 		if(status == VK_SUCCESS || overrideCheck){
					
		// 			vkFreeCommandBuffers(_device, pool, 1, &_singleTimeCommandBuffer[index].commandBuffer);
		// 			vkDestroyFence(_device, _singleTimeCommandBuffer[index].fence, nullptr);
		// 			_singleTimeCommandBuffer.erase(_singleTimeCommandBuffer.begin() + index);
		// 		}
		// 		else{
		// 			index++;
		// 		}
		// 	}
		// }

		Vk_DeviceLib::SingleTimeCommand vk_beginSingleTimeCommands(CommandCapabilities command) {
			return Vk_DeviceLib::beginSingleTimeCommands(
				_device,
				selectCommandPool(command),
				selectQueue(command)
			);

			// // // memory transfer operations are executed using command buffers
			// // // => allocate temporary command buffer
			// VkCommandBufferAllocateInfo allocInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
			// allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			// allocInfo.commandPool = selectCommandPool(command); // may want to specify another command pool for this kind of short lived command buffers
			// allocInfo.commandBufferCount = 1;
			// VkCommandBuffer buffer;
			// VK_CHECK(vkAllocateCommandBuffers(_device, &allocInfo, &buffer), "Unable to allocate single-time-command-buffer");

			// VkFenceCreateInfo fenceInfo{};
			// fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
			// // we must create the fences in the signaled state to ensure that on the first call to vkWaitForFences won't wait indefinitely
			// // fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT
			// VkFence fence;
			// VK_CHECK(vkCreateFence(_device, &fenceInfo, nullptr, &fence),"Failed to create synchronization objects for a frame!");

			// SingleTimeCommand singleTimeCommand = {
			// 	.commandBuffer=buffer, 
			// 	.fence=fence,
			// 	.command=command
			// };
			// //_singleTimeCommandBuffer.push_back(singleTimeCommand);

			// // memory transfer operations are executed using command buffers
			// // => allocate temporary command buffer
			// // VkCommandBufferAllocateInfo allocInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
			// // allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			// // allocInfo.commandPool = selectCommandPool(command); // may want to specify another command pool for this kind of short lived command buffers
			// // allocInfo.commandBufferCount = 1;

			// // VkCommandBuffer commandBuffer;
			// // vkAllocateCommandBuffers(_device, &allocInfo, &commandBuffer);

			// // start recoding the command
			// // VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT not necessary, because this is an execute and wait until finished kind
			// // of methode, therefore use
			// // VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT to tell the driver about our intentions
			// VkCommandBufferBeginInfo beginInfo = {};
			// beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			// beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

			// // VkResult status = vkGetFenceStatus(_device, _singleTimeFences[_singleTimeIndex]);

			// // std::cout << "##########################" << std::endl << " Wait for fence " << _singleTimeIndex << " (current status: " << static_cast<int64_t>(status) << ")" << std::endl;
			// // VkResult res = vkWaitForFences(_device, 1, &_singleTimeFences[_singleTimeIndex], VK_TRUE, UINT64_MAX);
			// // status = vkGetFenceStatus(_device, _singleTimeFences[_singleTimeIndex]);
			// // std::cout << "##########################" << std::endl << " got fence " << _singleTimeIndex << " (current status: " << static_cast<int64_t>(status) << ")" << std::endl;
			// // if(res == VK_TIMEOUT){
			// // 	Vk_Logger::RuntimeError(typeid(this), "Single time command timeout for index [{0}]", _singleTimeIndex);
			// // }
			// // else if (res != VK_SUCCESS) {
			// // 	Vk_Logger::RuntimeError(typeid(this), "Waiting for single time fence {0} had catastrphic result ({1})!", _singleTimeIndex, static_cast<int64_t>(res));
			// // }

			// // std::cout << "#############@@@##########" << std::endl << " Submit to fence " << _singleTimeIndex << std::endl;
			// // vkResetFences(_device, 1, &_singleTimeFences[_singleTimeIndex]);
			// // std::cout << "#############@@@##########" << std::endl << " reset fence " << _singleTimeIndex << std::endl;

			// vkBeginCommandBuffer(singleTimeCommand.commandBuffer, &beginInfo); // begin recording command

			// // singleTimeIndex = _singleTimeIndex;
			// // _singleTimeIndex++;
			// // _singleTimeIndex %= _singleTimeBufferCount;
			// // potentially select another command buffer
			// return singleTimeCommand;
		}

		void vk_endSingleTimeCommands(const Vk_DeviceLib::SingleTimeCommand& singleTimeCommand) {
			Vk_DeviceLib::endSingleTimeCommands(_device, singleTimeCommand);
		// 	vkEndCommandBuffer(singleTimeCommand.commandBuffer); // end recording command

		// 	// submit the command to the graphics queue
		// 	VkSubmitInfo submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
		// 	submitInfo.commandBufferCount = 1;
		// 	submitInfo.pCommandBuffers = &singleTimeCommand.commandBuffer;

		// 	auto t1 = std::chrono::high_resolution_clock::now();
		// 	// std::cout << "hello world" << std::endl;
		// 	VkResult res = Vk_ThreadSafe::Vk_ThreadSafe_QueueSubmit(selectQueue(singleTimeCommand.command), 1, &submitInfo, nullptr);
		// 	if(res != VK_SUCCESS){
		// 		if(res == VK_ERROR_OUT_OF_HOST_MEMORY){
		// 			Vk_Logger::RuntimeError(typeid(this), "vkQueueSubmit failed with VK_ERROR_OUT_OF_HOST_MEMORY ({0})!", static_cast<int64_t>(res));
		// 		}
		// 		else if(res == VK_ERROR_OUT_OF_DEVICE_MEMORY){
		// 			Vk_Logger::RuntimeError(typeid(this), "vkQueueSubmit failed with VK_ERROR_OUT_OF_DEVICE_MEMORY ({0})!", static_cast<int64_t>(res));
		// 		}
		// 		else{
		// 			Vk_Logger::RuntimeError(typeid(this), "vkQueueSubmit failed with {0}!", static_cast<int64_t>(res));
		// 		}
		// 	}
		// 	// Vk_ThreadSafe::Vk_ThreadSafe_QueueWaitIdle(_transferQueues[0]);
		// 	// auto t2 = std::chrono::high_resolution_clock::now();
		// 	// std::cout << "#############@@@##########" << std::endl << " command finished - " << std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count() << std::endl;
		// 	Vk_ThreadSafe::Vk_ThreadSafe_QueueWaitIdle(selectQueue(singleTimeCommand.command));

		// 	// res = vkWaitForFences(_device, 1, &singleTimeCommand.fence, VK_TRUE, UINT64_MAX);
		// 	// if(res == VK_TIMEOUT){
		// 	// 	Vk_Logger::RuntimeError(typeid(this), "Single time command timeout for index");
		// 	// }
		// 	// else if (res != VK_SUCCESS) {
		// 	// 	Vk_Logger::RuntimeError(typeid(this), "Waiting for single time fence had catastrphic result ({0})!", static_cast<int64_t>(res));
		// 	// }
		// 	// auto lock = std::lock_guard<std::mutex>(queue_submit_mutex);
		// 	// vk_cleanSingleTimeCommands(true);
		// 	// free command buffer memory
		// 	vkFreeCommandBuffers(_device, selectCommandPool(singleTimeCommand.command), 1, &singleTimeCommand.commandBuffer);
		// 	vkDestroyFence(_device, singleTimeCommand.fence, nullptr);
		}

		// void vk_transitionImageLayout(
		// 	CommandCapabilities command,
		// 	VkImage image,
		// 	VkFormat format,
		// 	VkImageLayout oldLayout,
		// 	VkImageLayout newLayout,
		// 	uint32_t mipLevels
		// ) {
		// 	// record command to move the image from buffer memory to real image memory where the shader
		// 	// can access it using a sampler
		// 	SingleTimeCommand singleTimeCommand = vk_beginSingleTimeCommands(command);

		// 	VkImageMemoryBarrier barrier = {};
		// 	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		// 	barrier.oldLayout = oldLayout;
		// 	barrier.newLayout = newLayout;
		// 	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED; // must use this if there is no transition between two differen queue families
		// 	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED; // must use this if there is no transition between two differen queue families
		// 	barrier.image = image;
		// 	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; // something funny, maybe find out what it's supposed to be
		// 	barrier.subresourceRange.baseMipLevel = 0; // mo mipmapping so only one basic layer with index 0
		// 	barrier.subresourceRange.levelCount = 1; // one level with texel colors
		// 	barrier.subresourceRange.baseArrayLayer = 0; // image is not formatted as an array
		// 	barrier.subresourceRange.layerCount = mipLevels; // one layer of texels

		// 	VkPipelineStageFlags sourceStage;
		// 	VkPipelineStageFlags destinationStage;

		// 	// initiate depth bufferig layout, newLayout can be used as initial layout because the initial contents of the depth image matter
		// 	if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
		// 		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

		// 		// check if has stencil component
		// 		bool hasStencilComponent =
		// 			(format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT);

		// 		if (hasStencilComponent) {
		// 			barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
		// 		}
		// 	}
		// 	else {
		// 		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		// 	}

		// 	if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
		// 		barrier.srcAccessMask = 0; // no need to wait for anything
		// 		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		// 		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		// 		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		// 	}
		// 	else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
		// 		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT; // wait until write operation is complete
		// 		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		// 		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		// 		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		// 	}
		// 	else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
		// 		barrier.srcAccessMask = 0;
		// 		barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		// 		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		// 		destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT; // reading happens here
		// 		// writing happens in VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT
		// 	}
		// 	else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
		// 		barrier.srcAccessMask = 0;
		// 		barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		// 		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		// 		destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		// 	}
		// 	else {
		// 		VK4::Vk_Logger::RuntimeError(typeid(this), "Unsupported layout transition!");
		// 	}

		// 	vkCmdPipelineBarrier(
		// 		singleTimeCommand.commandBuffer,
		// 		sourceStage, destinationStage,
		// 		0,
		// 		0, nullptr,
		// 		0, nullptr,
		// 		1, &barrier
		// 	);

		// 	vk_endSingleTimeCommands(singleTimeCommand);
		// }

		//inline VkFormat vk_findDepthFormat() 
		//{
		//	return vk_findSupportedTilingFormat(
		//		{ VK_FORMAT_D32_SFLOAT,
		//		  VK_FORMAT_D32_SFLOAT_S8_UINT,
		//		  VK_FORMAT_D24_UNORM_S8_UINT
		//		},
		//		VK_IMAGE_TILING_OPTIMAL,
		//		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT // apparently don't use vk_image_usage_... but vk_format_feature_...
		//	);
		//}

		//// find supported tiling formats out of a given list
		//inline VkFormat vk_findSupportedTilingFormat(
		//	const std::vector<VkFormat>& candidates,
		//	VkImageTiling tiling,
		//	VkFormatFeatureFlags features
		//) {
		//	// query the supported depth buffering formats
		//	for (VkFormat format : candidates) {
		//		VkFormatProperties props;

		//		vkGetPhysicalDeviceFormatProperties(
		//			_activePhysicalDevice->physicalDevice, format, &props
		//		);

		//		if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
		//			return format;
		//		}
		//		else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
		//			return format;
		//		}
		//	}

		//	VK4::Vk_Logger::RuntimeError(typeid(this), "falied to find supported tiling format!");

		//	return VK_FORMAT_UNDEFINED;
		//}

		Vk_Instance* vk_instance() {
			return _instance.get();
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

		// std::vector<Vk_DeviceLib::SingleTimeCommand> _singleTimeCommandBuffer;

		Vk_DevicePreference _devicePreference; 

		bool vk_register(uint64_t viewer){
			if(_viewer != 0 && _viewer != viewer) return false;
			_viewer = viewer;
		}

		void vk_unregister(){
			_viewer = 0;
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

		// inline uint32_t findMemoryType(
		// 	uint32_t typeFilter,
		// 	VkMemoryPropertyFlags properties
		// ) {
		// 	// two different types of entries:
		// 	// memoryTypes: different types of memory within heaps => array of VkMemoryType
		// 	// memoryHeaps: distinct memory resources like dedicated VRAM and swap in RAM where a heap is located may affect performance
		// 	VkPhysicalDeviceMemoryProperties memProperties;
		// 	vkGetPhysicalDeviceMemoryProperties(_activePhysicalDevice->physicalDevice, &memProperties);

		// 	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
		// 		// bitmask check the memory type
		// 		if (typeFilter & (1 << i) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
		// 			return i;
		// 		}
		// 	}

		// 	VK4::Vk_Logger::RuntimeError(typeid(this), "Failed to find suitable memory type!");

		// 	return 0;
		// }

		

		// uint32_t getMemoryTypeIndex(uint32_t typeBits, VkMemoryPropertyFlags properties) {
		// 	VkPhysicalDeviceMemoryProperties deviceMemoryProperties;
		// 	vkGetPhysicalDeviceMemoryProperties(_activePhysicalDevice->physicalDevice, &deviceMemoryProperties);
		// 	for (uint32_t i = 0; i < deviceMemoryProperties.memoryTypeCount; i++) {
		// 		if ((typeBits & 1) == 1) {
		// 			if ((deviceMemoryProperties.memoryTypes[i].propertyFlags & properties) == properties) {
		// 				return i;
		// 			}
		// 		}
		// 		typeBits >>= 1;
		// 	}
		// 	return 0;
		// }

		// VkBool32 getSupportedDepthFormat(VkFormat& depthFormat, Vk_DeviceLib::PhysicalDevice* pDev)
		// {
		// 	// Since all depth formats may be optional, we need to find a suitable depth format to use
		// 	// Start with the highest precision packed format
		// 	std::vector<VkFormat> depthFormats = {
		// 		VK_FORMAT_D32_SFLOAT_S8_UINT,
		// 		VK_FORMAT_D32_SFLOAT,
		// 		VK_FORMAT_D24_UNORM_S8_UINT,
		// 		VK_FORMAT_D16_UNORM_S8_UINT,
		// 		VK_FORMAT_D16_UNORM
		// 	};

		// 	for (auto& format : depthFormats)
		// 	{
		// 		VkFormatProperties formatProps;
		// 		vkGetPhysicalDeviceFormatProperties(pDev->physicalDevice, format, &formatProps);
		// 		// Format must support depth stencil attachment for optimal tiling
		// 		if (formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
		// 		{
		// 			depthFormat = format;
		// 			return true;
		// 		}
		// 	}

		// 	return false;
		// }

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

			// // if we added a new camera, we need to reaquire all subsequent details because they may have moved
			// // such that the address is no longer valid
			// // int camId = 0;
			// // if (camId >= 0) {
			// // 	if (_swapchainSupportDetailsUpToDate.find(camId) == _swapchainSupportDetailsUpToDate.end()) {
			// // 		_swapchainSupportDetailsUpToDate[camId] = false;
			// // 	}
			// // 	else if (_swapchainSupportDetailsUpToDate.at(camId))
			// // 		return _swapchainSupportDetails.at(camId);
			// // }

			// if(_swapchainSupportDetailsUpToDate){
			// 	return _swapchainSupportDetails;
			// }

			// if(surface == nullptr){
			// 	Vk_Logger::RuntimeError(typeid(this), "No surface passed to vk_swapchainSupport query!");
			// }

			// vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
			// 	pDev->physicalDevice, surface->vk_surface(), &_swapchainSupportDetails.capabilities
			// );

			// // query supported surface formats
			// uint32_t formatCount;
			// vkGetPhysicalDeviceSurfaceFormatsKHR(
			// 	pDev->physicalDevice, surface->vk_surface(), &formatCount, nullptr
			// );

			// // list all supported surface formats
			// if (formatCount != 0) {
			// 	_swapchainSupportDetails.formats.resize(formatCount);
			// 	vkGetPhysicalDeviceSurfaceFormatsKHR(
			// 		pDev->physicalDevice, surface->vk_surface(), &formatCount, _swapchainSupportDetails.formats.data()
			// 	);
			// }

			// // query all available surface presentation modes
			// uint32_t presentModeCount;
			// vkGetPhysicalDeviceSurfacePresentModesKHR(
			// 	pDev->physicalDevice, surface->vk_surface(), &presentModeCount, nullptr
			// );

			// // list all supported surface presentation modes
			// if (presentModeCount != 0) {
			// 	_swapchainSupportDetails.presentModes.resize(presentModeCount);
			// 	vkGetPhysicalDeviceSurfacePresentModesKHR(
			// 		pDev->physicalDevice, surface->vk_surface(), &presentModeCount, _swapchainSupportDetails.presentModes.data()
			// 	);
			// }

			// // get the capabilities for the size of the viewport
			// // =================================================
			// //auto& cap = _swapchainSupportDetails.capabilities;
			// //auto wh = surface->vk_surfaceExtent();

			// ////_swapchainSupportDetails
			// //if (cap.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
			// //	// if the maximal extend is not the maximal value of an uint32 then just take the current size in here
			// //	_swapchainSupportDetails.extent2d = cap.currentExtent;
			// //}
			// //else {
			// //	// otherwise make it as large as can possibly fit on the screen
			// //	// get the size of the window from glfw

			// //	VkExtent2D actualExtent = {
			// //		static_cast<uint32_t>(wh[0]),
			// //		static_cast<uint32_t>(wh[1])
			// //	};

			// //	actualExtent.width = std::max(cap.minImageExtent.width, std::min(cap.maxImageExtent.width, actualExtent.width));
			// //	actualExtent.height = std::max(cap.minImageExtent.height, std::min(cap.maxImageExtent.height, actualExtent.height));
			// //	_swapchainSupportDetails.extent2d = actualExtent;
			// //}

			// // get the possible surface format
			// // ===============================
			// // this is the best possibility
			// bool formatChosen = false;
			// auto& forms = _swapchainSupportDetails.formats;
			// if (forms.size() == 1 && forms[0].format == VK_FORMAT_UNDEFINED) {
			// 	_swapchainSupportDetails.surfaceFormat = { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
			// 	formatChosen = true;
			// }
			// else {
			// 	// if there are multiple formats available, check if the preferred format/colorspace combination is available
			// 	for (const auto& availableFormat : forms) {
			// 		if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			// 			_swapchainSupportDetails.surfaceFormat = availableFormat;
			// 			formatChosen = true;
			// 			break;
			// 		}
			// 	}
			// }
			// if(!formatChosen) {
			// 	// otherwise just return whatever is found
			// 	_swapchainSupportDetails.surfaceFormat = forms[0];
			// }

			// // get best supported depth format
			// // ================================
			// if (!getSupportedDepthFormat(_swapchainSupportDetails.depthFormat, pDev)) {
			// 	Vk_Logger::RuntimeError(typeid(this), GlobalCasters::castHighlightRed("No supported depth formats found!"));
			// }

			// // choose the possible present mode
			// // ================================
			// bool presentModeChosen = false;
			// auto& pModes = _swapchainSupportDetails.presentModes;
			// for (const auto& availablePresentMode : pModes) {
			// 	// VK_PRESENT_MODE_MAILBOX_KHR is the best one, use it if available
			// 	if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
			// 		_swapchainSupportDetails.selectedPresentMode = availablePresentMode;
			// 		presentModeChosen = true;
			// 		break;
			// 	}

			// 	// some drivers don't properly support fifo, so prefere this one if present
			// 	// with no mailbox
			// 	else if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
			// 		_swapchainSupportDetails.selectedPresentMode = availablePresentMode;
			// 		presentModeChosen = true;
			// 		break;
			// 	}
			// }
			// if (!presentModeChosen) {
			// 	_swapchainSupportDetails.selectedPresentMode = VK_PRESENT_MODE_FIFO_KHR;
			// }

			// // register if the current device supports wide lines
			// if (pDev->supportsWideLines()) {
			// 	_swapchainSupportDetails.supportsWideLines = true;
			// }

			// // if we require double buffering, check if that is possible
			// // =========================================================
			// // set the nFramesInFlight as either the N_FRAMES_IN_FLIGHT or whatever the hardware supports
			// if (_multiImageBuffering) {
			// 	// for all intents and purposes, this should be two
			// 	//_swapchainSupportDetails.nFramesInFlight = _swapchainSupportDetails.capabilities.minImageCount + 1;

			// 	// check if imageCount = 2 is too much... or else, keep imageCount = 2
			// 	const auto& caps = _swapchainSupportDetails.capabilities;
			// 	_swapchainSupportDetails.nFramesInFlight = std::max<uint32_t>(2, caps.minImageCount);
			// 	if (caps.maxImageCount > 0 && _swapchainSupportDetails.nFramesInFlight > caps.maxImageCount) {
			// 		Vk_Logger::RuntimeError(typeid(this), "Double Buffering not supported by GPU. Max supported Frames-In_Flight are " + caps.maxImageCount);
			// 		//_swapchainSupportDetails.nFramesInFlight = caps.maxImageCount;
			// 	}
			// }
			// else {
			// 	const auto& caps = _swapchainSupportDetails.capabilities;
			// 	_swapchainSupportDetails.nFramesInFlight = std::max<uint32_t>(1, caps.minImageCount);
			// 	if(caps.minImageCount > 1){
			// 		Vk_Logger::Warn(typeid(this), "Single image not supported by swapchain. Minimum " + std::to_string(caps.minImageCount) + " images required! Setting nFramesInFlight to "  + std::to_string(caps.minImageCount) + ".");
			// 	}
			// }

			// for(uint8_t i=0; i<_swapchainSupportDetails.nFramesInFlight; ++i){
			// 	auto q = std::queue<std::function<void()>>();
			// 	bridge.updates[i] = q;
			// }

			// // if (camId >= 0) {
			// // 	_swapchainSupportDetailsUpToDate[camId] = true;
			// // 	_swapchainSupportDetails[camId] = std::move(_swapchainSupportDetails);
			// // 	return _swapchainSupportDetails[camId];
			// // }
			// // else {
			// if(surface != nullptr){
			// 	// only set this to updated state if the query was made including a surface
			// 	// if this wasn't the case, the query will not retrieve the completed set of infos
			// 	// but also will not crash the program
			// 	_swapchainSupportDetailsUpToDate = true;
			// }
			// return _swapchainSupportDetails;
			// // }
		}

		// choose the best graphics supporting device out of all devices
		bool _setPhysicalDevice(Vk_Surface* surface)
		{
			return Vk_DeviceLib::setPhysicalDevice(_instance.get(), surface, _physicalDevices, _multiImageBuffering, _devicePreference, bridge, _physicalDeviceIndex, _activePhysicalDevice);
			// uint32_t deviceCount = 0;
			// VK_CHECK(
			// 	vkEnumeratePhysicalDevices(_instance->vk_instance(), &deviceCount, nullptr),
			// 	"Failed to enumerate physical devices"
			// );

			// if (deviceCount == 0)
			// 	Vk_Logger::RuntimeError(typeid(this), GlobalCasters::castHighlightRed("Failed to find devices with Vulkan support!"));

			// std::vector<VkPhysicalDevice> vkPhysicalDevices(deviceCount);
			// VkResult res = vkEnumeratePhysicalDevices(_instance->vk_instance(), &deviceCount, vkPhysicalDevices.data());
			// if(res == VK_INCOMPLETE){
			// 	std::string msg = "Fewer GPUs than actually presend in hardware were detected by the graphics driver. Note that in the newest version of Windows, the GPU is selected per App and all GPUs are visible by default. It can be that the vendor of one of the GPUs hides the respective other GPU. This causes VK_INCOMPLETE to be returned. If you use a dual GPU system, select the GPU that does not cause these problems and select the GPU for the viewer using Vk_DevicePreference.";
			// 	Vk_Logger::RuntimeError(typeid(this), msg);
			// }
			// else{
			// 	VK_CHECK(res, "Failed to load physical devices");
			// }

			// // iterate over all available physical devices to find a good one
			// int i = 0;
			// for (const auto& physicalDevice : vkPhysicalDevices) {
			// 	_physicalDevices.push_back(Vk_DeviceLib::PhysicalDevice());

			// 	// assign current physical device
			// 	_physicalDevices[i].physicalDevice = physicalDevice;

			// 	// query basic physical device stuff like type and supported vulkan version
			// 	vkGetPhysicalDeviceProperties(
			// 		_physicalDevices[i].physicalDevice,
			// 		&_physicalDevices[i].deviceProperties
			// 	);

			// 	// query exotic stuff about the device like viewport rendering or 64 bit floats
			// 	vkGetPhysicalDeviceFeatures(
			// 		_physicalDevices[i].physicalDevice,
			// 		&_physicalDevices[i].deviceFeatures
			// 	);

			// 	// get the maximum usable sample count
			// 	VkSampleCountFlags counts = std::min(
			// 		_physicalDevices[i].deviceProperties.limits.framebufferColorSampleCounts,
			// 		_physicalDevices[i].deviceProperties.limits.framebufferDepthSampleCounts
			// 	);

			// 	if (counts & VK_SAMPLE_COUNT_64_BIT) { _physicalDevices[i].maxUsableSampleCount = VK_SAMPLE_COUNT_64_BIT; }
			// 	else if (counts & VK_SAMPLE_COUNT_32_BIT) { _physicalDevices[i].maxUsableSampleCount = VK_SAMPLE_COUNT_32_BIT; }
			// 	else if (counts & VK_SAMPLE_COUNT_16_BIT) { _physicalDevices[i].maxUsableSampleCount = VK_SAMPLE_COUNT_16_BIT; }
			// 	else if (counts & VK_SAMPLE_COUNT_8_BIT) { _physicalDevices[i].maxUsableSampleCount = VK_SAMPLE_COUNT_8_BIT; }
			// 	else if (counts & VK_SAMPLE_COUNT_4_BIT) { _physicalDevices[i].maxUsableSampleCount = VK_SAMPLE_COUNT_4_BIT; }
			// 	else if (counts & VK_SAMPLE_COUNT_2_BIT) { _physicalDevices[i].maxUsableSampleCount = VK_SAMPLE_COUNT_2_BIT; }
			// 	else _physicalDevices[i].maxUsableSampleCount = VK_SAMPLE_COUNT_1_BIT;

			// 	// query device queue indices for graphics and present family
			// 	uint32_t queueFamilyCount = 0;
			// 	vkGetPhysicalDeviceQueueFamilyProperties(
			// 		_physicalDevices[i].physicalDevice,
			// 		&queueFamilyCount,
			// 		nullptr
			// 	);

			// 	_physicalDevices[i].queueFamilyProperties.resize(queueFamilyCount);
			// 	vkGetPhysicalDeviceQueueFamilyProperties(
			// 		_physicalDevices[i].physicalDevice,
			// 		&queueFamilyCount,
			// 		_physicalDevices[i].queueFamilyProperties.data()
			// 	);

			// 	// check for graphics and presentation support
			// 	VkBool32 presentSupport = false;
			// 	int index = 0;
			// 	for (const auto& queueFamily : _physicalDevices[i].queueFamilyProperties) {
			// 		// check if given device supports graphics
			// 		if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT && _physicalDevices[i].queueFamilyIndices.graphicsFamilyIndex < 0) {
			// 			_physicalDevices[i].queueFamilyIndices.graphicsFamilyIndex = index;
			// 		}

			// 		// check if given device supports presentation to a particular given surface
			// 		vkGetPhysicalDeviceSurfaceSupportKHR(_physicalDevices[i].physicalDevice, index, surface->vk_surface(), &presentSupport);
			// 		if (queueFamily.queueCount > 0 && presentSupport && _physicalDevices[i].queueFamilyIndices.presentFamilyIndex < 0) {
			// 			_physicalDevices[i].queueFamilyIndices.presentFamilyIndex = index;
			// 		}

			// 		if(queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT){
			// 			_physicalDevices[i].queueFamilyIndices.transferFamilyIndex = index;
			// 		}
			// 		index++;
			// 	}

			// 	// if both queues have an index that is greater than 0, break
			// 	if (!_physicalDevices[i].queueFamilyIndices.isComplete()) {
			// 		Vk_Logger::RuntimeError(typeid(this), "No suitable graphics device found");
			// 	}

			// 	// query all available extensions
			// 	uint32_t extensionCount;
			// 	vkEnumerateDeviceExtensionProperties(
			// 		_physicalDevices[i].physicalDevice,
			// 		nullptr,
			// 		&extensionCount,
			// 		nullptr
			// 	);

			// 	_physicalDevices[i].availableExtensions.resize(extensionCount);
			// 	vkEnumerateDeviceExtensionProperties(
			// 		_physicalDevices[i].physicalDevice,
			// 		nullptr,
			// 		&extensionCount,
			// 		_physicalDevices[i].availableExtensions.data()
			// 	);

			// 	i++;
			// }

			// // check to see if there is one physical device suitable for graphics application
			// std::vector<Vk_DeviceLib::PhysicalDevice>::iterator pDev = _physicalDevices.begin();
			// int index = 0;

			// // prefer dedicated GPU vs integrated GPU
			// bool found = false;
			// while (pDev != _physicalDevices.end()) {
			// 	const SwapchainSupportDetails& support = vk_swapchainSupport(&(*pDev), surface);
			// 	found = pDev->supportsGraphicsQueue()
			// 		&& pDev->supportsPresentationQueue()
			// 		&& pDev->supportsSwapchainExtension()
			// 		&& pDev->supportsMemoryBudgetExtension()
			// 		&& !support.formats.empty()
			// 		&& !support.presentModes.empty()
			// 		&& pDev->deviceFeatures.samplerAnisotropy
			// 		&& findGpuForDevicePreferences(*pDev)
			// 		&& pDev->deviceFeatures.geometryShader;

			// 	if (found) {
			// 		_physicalDeviceIndex = index;
			// 		_activePhysicalDevice = &_physicalDevices.at(index);
			// 		return true;
			// 	}

			// 	index++;
			// 	pDev++;
			// }

			// return false;
		}

		// bool findGpuForDevicePreferences(const Vk_DeviceLib::PhysicalDevice& physicalDevice){
		// 	if(_devicePreference == Vk_DevicePreference::USE_ANY_GPU){
		// 		return (
		// 			(physicalDevice.deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) || 
		// 			(physicalDevice.deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
		// 		);
		// 	}

		// 	if(_devicePreference == Vk_DevicePreference::USE_DISCRETE_GPU){
		// 		return (
		// 			(physicalDevice.deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
		// 		);
		// 	}

		// 	if(_devicePreference == Vk_DevicePreference::USE_INTEGRATED_GPU){
		// 		return ( 
		// 			(physicalDevice.deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
		// 		);
		// 	}

		// 	return false;
		// }

		// void updateGpuHeapUsageStats() {
			// VkPhysicalDeviceMemoryProperties2 props;
			// props.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2;

			// VkPhysicalDeviceMemoryBudgetPropertiesEXT next;
			// next.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_BUDGET_PROPERTIES_EXT;
			// next.pNext = VK_NULL_HANDLE;
			// props.pNext = &next;

			// for(auto& device : _physicalDevices) {
			// 	vkGetPhysicalDeviceMemoryProperties2(device.physicalDevice, &props);

			// 	for (uint32_t i = 0; i < props.memoryProperties.memoryHeapCount; ++i) {
			// 		double budget = static_cast<double>(next.heapBudget[i]);
			// 		double budget_kb = round(budget / 1.0e3, 2);
			// 		double budget_mb = round(budget / 1.0e6, 2);
			// 		double budget_gb = round(budget / 1.0e9, 2);
			// 		_gpuHeapConfig[&device][i].heapBudget = next.heapBudget[i];
			// 		_gpuHeapConfig[&device][i].heapBudgetKb = budget_kb;
			// 		_gpuHeapConfig[&device][i].heapBudgetMb = budget_mb;
			// 		_gpuHeapConfig[&device][i].heapBudgetGb = budget_gb;

			// 		double usage = static_cast<double>(next.heapUsage[i]);
			// 		double usage_kb = round(usage / 1.0e3, 2);
			// 		double usage_mb = round(usage / 1.0e6, 2);
			// 		double usage_gb = round(usage / 1.0e9, 2);
			// 		_gpuHeapConfig[&device][i].heapUsage = next.heapUsage[i];
			// 		_gpuHeapConfig[&device][i].heapUsageKb = usage_kb;
			// 		_gpuHeapConfig[&device][i].heapUsageMb = usage_mb;
			// 		_gpuHeapConfig[&device][i].heapUsageGb = usage_gb;
			// 	}
			// }
		// }

		void setGpuMemoryConfig() {
			Vk_DeviceLib::setGpuMemoryConfig(_physicalDevices, _gpuHeapConfig, _gpuMemoryConfig);
			// for (auto& device : _physicalDevices) {
			// 	VkPhysicalDeviceMemoryProperties deviceMemoryProperties;
			// 	vkGetPhysicalDeviceMemoryProperties(_activePhysicalDevice->physicalDevice, &deviceMemoryProperties);

			// 	for (uint32_t i = 0; i < deviceMemoryProperties.memoryHeapCount; ++i) {
			// 		double s = static_cast<double>(deviceMemoryProperties.memoryHeaps[i].size);
			// 		double s_kb = Vk_Lib::round(s / 1.0e3, 2);
			// 		double s_mb = Vk_Lib::round(s / 1.0e6, 2);
			// 		double s_gb = Vk_Lib::round(s / 1.0e9, 2);
			// 		_gpuHeapConfig[&device].push_back(GpuMemoryHeapConfiguration{
			// 			.heapSize = deviceMemoryProperties.memoryHeaps[i].size,
			// 			.heapSizeKb = s_kb,
			// 			.heapSizeMb = s_mb,
			// 			.heapSizeGb = s_gb,
			// 			.heapIndex = static_cast<uint32_t>(i),
			// 			.heapConfigs = std::vector<GpuMemoryConfiguration*>()
			// 			});
			// 	}

			// 	for (uint32_t i = 0; i < deviceMemoryProperties.memoryTypeCount; ++i) {
			// 		auto h = deviceMemoryProperties.memoryTypes[i];
			// 		std::string propStrShort;
			// 		std::string propStrLong;
			// 		devicePropertyFlagToString(h.propertyFlags, propStrShort, propStrLong);

			// 		_gpuMemoryConfig[&device].insert({
			// 			h.propertyFlags,
			// 			GpuMemoryConfiguration{
			// 				.heapIndex = h.heapIndex,
			// 				.flagsStr = propStrShort,
			// 				.flagsStrLong = propStrLong
			// 			}
			// 			});

			// 		_gpuHeapConfig[&device].at(h.heapIndex).heapConfigs.push_back(&_gpuMemoryConfig[&device].at(h.propertyFlags));
			// 	}
			// }
		}

		void devicePropertyFlagToString(const VkMemoryPropertyFlags flags, std::string& propStrShort, std::string& propStrLong) {
			Vk_DeviceLib::devicePropertyFlagToString(flags, propStrShort, propStrLong);
			// std::string type = "";
			// switch (flags) {
			// case 0:  
			// 	type = "HOST LOCAL / SHARED"; 
			// 	break;
			// case VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT:
			// 	type = "VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT"; 
			// 	break;
			// case VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT:
			// 	type = "VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT";
			// 	break;
			// case VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT:
			// 	type = "VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT"; 
			// 	break;
			// case VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT:
			// 	type = "VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT"; 
			// 	break;
			// case VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT:
			// 	type = "VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT"; 
			// 	break;
			// case VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT:
			// 	type = "VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT";
			// 	break;
			// case VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT:  
			// 	type = "VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT"; 
			// 	break;
			// case VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT:  
			// 	type = "VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT"; 
			// 	break;
			// case VK_MEMORY_PROPERTY_PROTECTED_BIT:  
			// 	type = "VK_MEMORY_PROPERTY_PROTECTED_BIT"; 
			// 	break;
			// case VK_MEMORY_PROPERTY_PROTECTED_BIT | VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT: 
			// 	type = "VK_MEMORY_PROPERTY_PROTECTED_BIT | VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT"; 
			// 	break;
			// case VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD: 
			// 	type = "VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD"; 
			// 	break;
			// case VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD: 
			// 	type = "VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD"; 
			// 	break;
			// case VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD: 
			// 	type = "VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD"; 
			// 	break;
			// case VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD: 
			// 	type = "VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD"; 
			// 	break;
			// case VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD: 
			// 	type = "VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD"; 
			// 	break;
			// case VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD | VK_MEMORY_PROPERTY_DEVICE_UNCACHED_BIT_AMD: 
			// 	type = "VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD | VK_MEMORY_PROPERTY_DEVICE_UNCACHED_BIT_AMD"; 
			// 	break;
			// case VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD | VK_MEMORY_PROPERTY_DEVICE_UNCACHED_BIT_AMD: 
			// 	type = "VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD | VK_MEMORY_PROPERTY_DEVICE_UNCACHED_BIT_AMD"; 
			// 	break;
			// case VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD | VK_MEMORY_PROPERTY_DEVICE_UNCACHED_BIT_AMD: 
			// 	type = "VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD | VK_MEMORY_PROPERTY_DEVICE_UNCACHED_BIT_AMD"; 
			// 	break;
			// case VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD | VK_MEMORY_PROPERTY_DEVICE_UNCACHED_BIT_AMD: 
			// 	type = "VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD | VK_MEMORY_PROPERTY_DEVICE_UNCACHED_BIT_AMD"; 
			// 	break;
			// case VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD | VK_MEMORY_PROPERTY_DEVICE_UNCACHED_BIT_AMD: 
			// 	type = "VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD | VK_MEMORY_PROPERTY_DEVICE_UNCACHED_BIT_AMD"; 
			// 	break;
			// case VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_RDMA_CAPABLE_BIT_NV: 
			// 	type = "VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_RDMA_CAPABLE_BIT_NV"; 
			// 	break;
			// default: break;
			// }

			// propStrLong = std::string(type);

			// while (Vk_Lib::replace(type, "VK_MEMORY_PROPERTY_", "")) {};
			// while (Vk_Lib::replace(type, "_BIT", "")) {};
			// while (Vk_Lib::replace(type, "_", " ")) {};

			// propStrShort = std::string(type);
		}

		
	};
}
