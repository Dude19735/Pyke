#pragma once

// #include <vulkan/vulkan.h>
#include <vector>
#include <string>

#include "../../../Defines.h"
#include "Vk_Swapchain_IM.hpp"
#include "../../I_RenderPass.hpp"

namespace VK4 {

	class Vk_RenderPass_IM : public I_RenderPass {
		/**
		* Note: the colorBuffer and depthBuffer are not actually used here.
		*       They are only required to check for their presence
		*/
	public:
		Vk_RenderPass_IM(
			Vk_Device* const device,
			Vk_Surface* surface
		) : 
			_device(device),
			_surface(surface)
		{
			VK4::Vk_Logger::Log(typeid(this), GlobalCasters::castConstructorTitle("Create IM Renderpass"));
			
			const SwapchainSupportDetails& details = _device->vk_swapchainSupportActiveDevice(_surface);
			VkFormat swapchainImageFormat = details.surfaceFormat.format;
			VkFormat swapchainDepthFormat = details.depthFormat;
			VkSampleCountFlagBits maxUsableSampleCount = _device->vk_sampleCount();

			// describe subpass
			// add reference to the attachment for the first and only subpass
			VkSubpassDescription subpass = {};
			// specify it's a graphics and not compute subpass
			subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS; 

			std::uint32_t attachmentLocation = 0;

			//!NOTE: the attachment with VK_IMAGE_LAYOUT_PRESENT_SRC_KHR as finalLayoutVK_DYNAMIC_STATE_SCISSOR
			//!      has to be mapped onto the swapchain image views

			// first, add depth filtering if we have a depth buffer
			VkAttachmentDescription depthAttachment = {}; // how to mix old and new depth buffer data
			depthAttachment.format = swapchainDepthFormat; //_device->vk_findDepthFormat(); // must be the same as depth image format
			depthAttachment.samples = maxUsableSampleCount; //_device->vk_getSampleCount(); // use multisampling
			depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; // what to do before any ops on buffer are performed
			depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE; // store rendering data after one render pass? => don't care, don't show depth buffer
			depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // the previous depth contents don't matter => undefined
			depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

			// create a reference to the depth attachment to add to the first and only subpass
			VkAttachmentReference depthAttachmentRef = {};
			depthAttachmentRef.attachment = attachmentLocation;
			depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

			//- pDepthStencilAttachment : Attachments for depth and stencil data
			subpass.pDepthStencilAttachment = &depthAttachmentRef;

			attachmentLocation++;

			// then add the normal color image that, without multisampling will be the one mapped onto the swapchain image view
			VkAttachmentDescription colorAttachment = {};
			colorAttachment.format = swapchainImageFormat; // match format of swapchain images
			colorAttachment.samples = maxUsableSampleCount; //_device->vk_getSampleCount(); // use multisampling
			colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; // load, clear or don't care about existing contents of the attachement before loading
			colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE; // want to see something on the screen after rendering it? => preserve/store content
			colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE; // no stencil buffer used
			colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // layout before current operation (don't care)
			
			colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // layout after current operation (VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL instead of VK_IMAGE_LAYOUT_PRESENT_SRC_KHR for multisampled images)
			//colorAttachment.finalLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;

			// use subpasses to group rendering operations and maybe optimize the process
			VkAttachmentReference colorAttachmentRef = {};
			colorAttachmentRef.attachment = attachmentLocation; // where to bind: layout(location = 0) out vec4 outColor
			colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // layout attachment should have during subpass

			//- pInputAttachments: Attachments that are read from a shader
			subpass.colorAttachmentCount = 1;
			subpass.pColorAttachments = &colorAttachmentRef;

			attachmentLocation++;

			// then add the possible image multisampling resolve that will replace the normal color attachment as last if present
			VkAttachmentDescription colorAttachmentResolve = {};
			// tell the GPU how to resolve an oversampled image down to something presentable
			// how to reduce multisampled image
			colorAttachmentResolve.format = swapchainImageFormat;
			colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
			colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // use presentable format

			// create a new reference to the color attachment
			VkAttachmentReference colorAttachmentResolveRef = {};
			colorAttachmentResolveRef.attachment = attachmentLocation;
			colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			//- pResolveAttachments : Attachments used for multisampling color attachments
			subpass.pResolveAttachments = &colorAttachmentResolveRef;

			attachmentLocation++;

			// add the dependency to the renderPassInfo
			// the createinfo for the render pass contains all 
			// the render pass will now wait until the color attachment stage is finished
			//! the sequence of attachment in renderPass and frameBuffers has to match!
			//! * first is always the depthBuffer, if present
			//! * then is the color attachment with or without multisampling
			//!    + will map in frameBuffer to swapchainImageViews if no multisampling colorBuffer is present
			//!    + will map to the colorBuffer if multisampling resolve is present
			//! * last is the colorBuffer
			//!    + if no extra buffer is present, there is no multisampling
			//!    + the color attachment will attach to the swapchainImageViews
			//!    + if an extra colorBuffer is present, the color attachment maps to the colorBuffer
			//!      and the sampling resolve to the swapchainImageView
			std::vector<VkAttachmentDescription> attachments;
			attachments.push_back(depthAttachment);
			attachments.push_back(colorAttachment);
			attachments.push_back(colorAttachmentResolve);
			
			//- pPreserveAttachments : Attachments that are not used by this subpass, but for which the data must be preserved

			VkSubpassDependency dependency = {};
			// there are two possibilities to make the render pass wait until the image is finished:
			// 1. wait for the entire pipeline to finish
			// 2. make the subpass wait for the color attachment output
			// make sure, that dstSubpass id allways higher than srcSubpass => alternative is cyclic dependence of waits
			dependency.srcSubpass = VK_SUBPASS_EXTERNAL; // refere to implicit subpass before or after explicitly defined subpasses
			// index of explicitly defined subpass (one subpass => index 0)
			dependency.dstSubpass = 0;
			dependency.srcAccessMask = 0;
			// specify the operations to wait for and the stage in which they occur
			dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			// specify the operations that should wait for the attachment output bit wait before continuation until both dstAccessMask bits are set
			dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

			VkRenderPassCreateInfo renderPassInfo = { VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
			renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
			renderPassInfo.pAttachments = attachments.data();
			renderPassInfo.subpassCount = 1;
			renderPassInfo.pSubpasses = &subpass;
			renderPassInfo.dependencyCount = 1;
			renderPassInfo.pDependencies = &dependency;

			VK_CHECK(vkCreateRenderPass(_device->vk_lDev(), &renderPassInfo, nullptr, &_renderPass), "Failed to create render pass");
		}
		
		~Vk_RenderPass_IM()
		{
			VK4::Vk_Logger::Log(typeid(this), GlobalCasters::castDestructorTitle("Destroy Renderpass"));
			vkDestroyRenderPass(_device->vk_lDev(), _renderPass, nullptr);
		}

		const VkRenderPass vk_renderPass() const { return _renderPass; }

		private:
			Vk_Device* _device;
			Vk_Surface* _surface;

			VkRenderPass _renderPass;
	};
}