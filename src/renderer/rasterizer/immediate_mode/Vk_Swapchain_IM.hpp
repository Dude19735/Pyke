#pragma once

// #include <vulkan/vulkan.h>
#include <vector>

#include "../../../Defines.h"
#include "../../I_Swapchain.hpp"
#include "../../../application/Vk_Device.hpp"

namespace VK4 {

	class Vk_Swapchain_IM : public I_Swapchain {

		/* 
		* A swapchain is a queue of images that are waiting to be
		* presented to the screen.
		* 
		* The swapchain is not really very different for different
		* renderer complexities. The command buffers are what changes things
		*/

	public:
		Vk_Swapchain_IM() = delete;
		Vk_Swapchain_IM(Vk_Device* const device, Vk_Surface* surface)
			:
			_device(device),
			_surface(surface),
			_swapchain(VK_NULL_HANDLE)

		{
			VK4::Vk_Logger::Log(typeid(this), GlobalCasters::castConstructorTitle("Create Swapchain"));

			const SwapchainSupportDetails& details = _device->vk_swapchainSupportActiveDevice(_surface);

			auto& caps = details.capabilities;
			// decide on size of image queue (this one means 2 images)
			_swapchainImageCount = details.nFramesInFlight;

			// VkExtent2D extent2d = surface->vk_surfaceExtent();
			VkExtent2D extent2d = details.capabilities.currentExtent;

			// create swapchain info struct
			VkSwapchainCreateInfoKHR swapchainCreateInfo = { VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
			swapchainCreateInfo.surface = _surface->vk_surface();
			swapchainCreateInfo.minImageCount = details.nFramesInFlight;
			swapchainCreateInfo.imageFormat = details.surfaceFormat.format;
			swapchainCreateInfo.imageColorSpace = details.surfaceFormat.colorSpace;
			swapchainCreateInfo.imageExtent = extent2d;
			swapchainCreateInfo.imageArrayLayers = 1;
			swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

			// how to handle swap chain images that will be used accross multiple queue families
			// in case graphics queue is different from presentation queue
			
			uint32_t queueFamilyIndices[] = { _device->vk_graphicsFamilyIndex(), _device->vk_presentFamilyIndex() };

			if (queueFamilyIndices[0] != queueFamilyIndices[1]) {
				// share the image between the queues
				swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
				swapchainCreateInfo.queueFamilyIndexCount = 2;
				// concurrent mode requires specification between which queues sharing will occur
				swapchainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;
			}
			else {
				// no need to pass additional information
				swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
				swapchainCreateInfo.queueFamilyIndexCount = 0;
				swapchainCreateInfo.pQueueFamilyIndices = nullptr;
			}

			// may flip the image
			swapchainCreateInfo.preTransform = caps.currentTransform;	

			if (caps.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR) {
				// if the swapchain supports transparency
				swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR;
			}
			else {
				// may use alpha or not
				swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
			}

			swapchainCreateInfo.presentMode = details.selectedPresentMode;
			// dont care about obscured pixels
			swapchainCreateInfo.clipped = VK_TRUE;

			// handle for an invalid swap chain
			swapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE;						

			VkDevice lDev = _device->vk_lDev();
			VkResult res = vkCreateSwapchainKHR(lDev, &swapchainCreateInfo, nullptr, &_swapchain);
			VK_CHECK(res, "Failed to create swapchain");

			// retreive images from swap chain (basically map the memory into a vector)
			VkResult result = vkGetSwapchainImagesKHR(lDev, _swapchain, &_swapchainImageCount, nullptr);
			VK_CHECK(result, "Failed to get swapchain");

			_swapchainImages.resize(_swapchainImageCount);
			vkGetSwapchainImagesKHR(lDev, _swapchain, &_swapchainImageCount, _swapchainImages.data());
			_device->vk_testAndUpdateNFramesInFlight(_swapchainImageCount);

			// create image views for all the images
			createSwapchainImageViews(
				lDev,
				_swapchainImages,
				details.surfaceFormat.format,
				_swapchainImageViews
			);

			// create color buffer
			_device->vk_createSwapchainResource(
				VK_IMAGE_TILING_OPTIMAL,
				VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				VK_IMAGE_ASPECT_COLOR_BIT,
				extent2d,
				details.surfaceFormat.format,
				1,
				VK_IMAGE_LAYOUT_UNDEFINED,
				VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
				_colorImage,
				_colorImageMemory,
				_colorImageView
			);

			_device->vk_createSwapchainResource(
				VK_IMAGE_TILING_OPTIMAL,
				VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				VK_IMAGE_ASPECT_DEPTH_BIT,
				extent2d,
				details.depthFormat,
				1,
				VK_IMAGE_LAYOUT_UNDEFINED,
				VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
				_depthImage,
				_depthImageMemory,
				_depthImageView
			);

			_device->vk_createBuffer(
				VK_BUFFER_USAGE_TRANSFER_DST_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT,
				_dstColor1Buffer,
				_dstColor1BufferMemory,
				static_cast<VkDeviceSize>(caps.currentExtent.width * caps.currentExtent.height * sizeof(float) * 4)
			);

			VkExtent3D imageExtent = VkExtent3D{
				static_cast<uint32_t>(extent2d.width),
				static_cast<uint32_t>(extent2d.height),
				1
			};

			createCopyCommands(imageExtent);
			_color1.resize(details.capabilities.currentExtent.width * details.capabilities.currentExtent.height * 4);
		}

		~Vk_Swapchain_IM()
		{
			VK4::Vk_Logger::Log(typeid(this), GlobalCasters::castDestructorTitle("Destroy Swapchain"));

			VkDevice cLDev = _device->vk_lDev();

			vkDestroySwapchainKHR(cLDev, _swapchain, nullptr);

			vkDestroyBuffer(cLDev, _dstColor1Buffer, nullptr);
			vkFreeMemory(cLDev, _dstColor1BufferMemory, nullptr);

			std::vector<VkImageView>::iterator iter = _swapchainImageViews.begin();
			std::vector<VkImageView>::iterator end = _swapchainImageViews.end();
			while (iter != end) {
				vkDestroyImageView(cLDev, *iter, nullptr);
				++iter;
			}

			vkDestroyImageView(cLDev, _colorImageView, nullptr);
			vkDestroyImage(cLDev, _colorImage, nullptr);
			vkFreeMemory(cLDev, _colorImageMemory, nullptr);

			vkDestroyImageView(cLDev, _depthImageView, nullptr);
			vkDestroyImage(cLDev, _depthImage, nullptr);
			vkFreeMemory(cLDev, _depthImageMemory, nullptr);
		}

		const Screenshot vk_getFrameBuffer(int index) {
			VkExtent2D extent2d = _device->vk_swapchainSupportActiveDevice(_surface).capabilities.currentExtent;

			int width = static_cast<int>(extent2d.width);
			int height = static_cast<int>(extent2d.height);

			_device->vk_submitWork(_copyCmd[index]);

			VkDeviceSize size = static_cast<VkDeviceSize>(width * height * sizeof(uint8_t) * 4);
			_device->vk_copyDeviceBufferToVector(_color1.data(), _dstColor1BufferMemory, size);
			VK4::Vk_Logger::Log(typeid(this), "Framebuffer color 1 saved to vector");

			return Screenshot {
				.color=_color1,
				.height=height,
				.width=width
			};
		}

		const VkSwapchainKHR vk_swapchain() const { return _swapchain; }
		const std::vector<VkImageView>& vk_imageViews() const { return _swapchainImageViews; }
		const VkImageView vk_depthImageView() const { return _depthImageView; }
		const VkImageView vk_colorImageView() const { return _colorImageView; }

	private:
		std::uint32_t _swapchainImageCount;
		Vk_Device* _device;
		Vk_Surface* _surface;
		
		// multisampling color buffer
		VkImage _colorImage;
		VkDeviceMemory _colorImageMemory;
		VkImageView _colorImageView;

		//! depth image
		VkImage _depthImage;
		VkDeviceMemory _depthImageMemory;
		VkImageView _depthImageView;

		//! swapchain
		VkSwapchainKHR _swapchain;
		std::vector<VkImage> _swapchainImages;
		std::vector<VkImageView> _swapchainImageViews;

		std::vector<VkCommandBuffer> _copyCmd;
		VkBuffer _dstColor1Buffer;
		VkDeviceMemory _dstColor1BufferMemory;
		std::vector<uint8_t> _color1;

		inline void createSwapchainImageViews(
			const VkDevice device,
			const std::vector<VkImage>& swapchainImages,
			const VkFormat swapchainImageFormat,
			std::vector<VkImageView>& swapchainImageViews
		) {
			swapchainImageViews.resize(swapchainImages.size());

			for (size_t i = 0; i < swapchainImages.size(); i++) {
				swapchainImageViews[i] = _device->vk_createImageView(
					swapchainImages[i],
					swapchainImageFormat,
					VK_IMAGE_ASPECT_COLOR_BIT,
					1
				);
			}
		}

		void createCopyCommands(VkExtent3D imageExtent) {
			_copyCmd.clear();
			_copyCmd.resize(_swapchainImageCount);

			int scic = static_cast<int>(_swapchainImageCount);
			for (int i = 0; i < scic; ++i) {
				_device->vk_createCopyImgToBufferCommand(_copyCmd[i], _swapchainImages[i], _dstColor1Buffer, imageExtent);
			}
		}
	};
}