#pragma once

#include "../Defines.h"

namespace VK4 {
	class I_Swapchain {
	public:
		virtual ~I_Swapchain() {};
		virtual const VkSwapchainKHR vk_swapchain() const = 0;
		virtual const std::vector<VkImageView>& vk_imageViews() const = 0;
		virtual const VkImageView vk_depthImageView() const = 0;
		virtual const VkImageView vk_colorImageView() const = 0;
		virtual const Screenshot vk_getFrameBuffer(int index) = 0;
	};
}