#pragma once

#include <vector>

#include "../../../Defines.h"
#include "../../../application/Vk_Device.hpp"
#include "Vk_RenderPass_IM.hpp"
#include "Vk_Swapchain_IM.hpp"
#include "../../I_FrameBuffer.hpp"
#include "../../I_RenderPass.hpp"

namespace VK4 {

	class Vk_Framebuffer_IM : public I_FrameBuffer {

	public:
		Vk_Framebuffer_IM() = delete;
		Vk_Framebuffer_IM(
			Vk_Device* const device,
			Vk_Surface* surface,
			I_Swapchain* swapchain,
			I_RenderPass* renderPass
		) :
			_device(device),
			_surface(surface),
			_swapchain(swapchain),
			_renderPass(renderPass),
			_framebuffers(std::vector<VkFramebuffer>())
		{
			VK4::Vk_Logger::Log(typeid(this), GlobalCasters::castConstructorTitle("Create Framebuffer IM"));

			uint32_t imageViewLayerCount = 1; // this can be used for multilayered images (no need for that now)
			VkExtent2D extent2d = _device->vk_swapchainSupportActiveDevice(_surface).capabilities.currentExtent;

			auto swapchainImageViews = _swapchain->vk_imageViews();
			_framebuffers.resize(swapchainImageViews.size());


			// create framebuffers for all image views
			// we only have one attachement for every image
			for (size_t i = 0; i < swapchainImageViews.size(); i++) {
				std::vector<VkImageView> attachments;
				attachments.push_back(_swapchain->vk_depthImageView());
				attachments.push_back(_swapchain->vk_colorImageView());
				attachments.push_back(swapchainImageViews[i]);
				
				VkFramebufferCreateInfo framebufferInfo = { VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
				// renderpass must be compatible, use same number and type of attachments
				framebufferInfo.renderPass = _renderPass->vk_renderPass();
				framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
				framebufferInfo.pAttachments = attachments.data();
				framebufferInfo.width = extent2d.width;
				framebufferInfo.height = extent2d.height;
				// amount of layers in image views => single images => 1 layer
				framebufferInfo.layers = imageViewLayerCount;

				VK_CHECK(vkCreateFramebuffer(_device->vk_lDev(), &framebufferInfo, nullptr, &_framebuffers[i]), "Failed to create frame buffer");
			}
		}

		~Vk_Framebuffer_IM()
		{
			VK4::Vk_Logger::Log(typeid(this), GlobalCasters::castDestructorTitle("Destroy Framebuffer"));

			VkDevice cLDev = _device->vk_lDev();

			std::vector<VkFramebuffer>::iterator iter = _framebuffers.begin();
			std::vector<VkFramebuffer>::iterator end = _framebuffers.end();
			while (iter != end) {
				vkDestroyFramebuffer(cLDev, *iter, nullptr);
				++iter;
			}
		}

		std::vector<VkFramebuffer>* vk_frameBuffers() {
			return &_framebuffers;
		}

	private:

		Vk_Device* _device;
		Vk_Surface* _surface;
		I_Swapchain* _swapchain;
		I_RenderPass* _renderPass;

		//! frame buffers
		std::vector<VkFramebuffer> _framebuffers;
	};
}