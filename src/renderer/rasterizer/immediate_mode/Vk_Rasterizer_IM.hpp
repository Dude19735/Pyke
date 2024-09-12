#pragma once

#include "../../../Defines.h"
#include "../../I_Renderer.hpp"

#include "Vk_Graphicspipeline_IM.hpp"
#include "Vk_Framebuffer_IM.hpp"
#include "Vk_RenderPass_IM.hpp"

#include "../../../objects/dot/S_Dot_P_C.hpp"
#include "../../../objects/line/S_Line_P_C.hpp"
#include "../../../objects/mesh/S_Mesh_P_C.hpp"

namespace VK4 {
	class Vk_Rasterizer_IM : public I_Renderer {
	public:
		Vk_Rasterizer_IM(
			// LWWS::TViewportId viewportId,
			Vk_Device* const device,
			// I_Renderer::Vk_PipelineAuxilliaries auxilliaries,
			const Vk_SurfaceConfig& surfaceConfig,
			int freshPoolSize,
			UniformBufferType_RendererMat4 mvpInit
		)
			:
			I_Renderer(
				//I_Renderer::Caster{
				//	.p = std::addressof(*this),
				//	.cast = [](void* x) { static_cast<Vk_Rasterizer_IM*>(x)->Destroy(); }
				//},
				// viewportId, 
				device, 
				// auxilliaries,
				surfaceConfig,
				freshPoolSize, 
				mvpInit
			),
			_renderpass(nullptr),
			_swapchain(nullptr),
			_framebuffer(nullptr),
			_commandBuffer({}),
			_imageAvailableSemaphores({}),
			_renderFinishedSemaphores({}),
			_inFlightFences({})
		{
			VK4::Vk_Logger::Log(typeid(this), GlobalCasters::castConstructorTitle("Create Vk_Rasterizer_IM"));
			createSyncResources();
			allocateCommandBuffers(_device, _surfaceConfig, _device->vk_renderingCommandPool(), _commandBuffer);

			_renderpass = std::make_unique<Vk_RenderPass_IM>(_device, _surfaceConfig);
			_swapchain = std::make_unique<Vk_Swapchain_IM>(_device, _surfaceConfig);
			_framebuffer = std::make_unique<Vk_Framebuffer_IM>(_device, _surfaceConfig, _swapchain.get(), _renderpass.get());
			
			//dtors.push_back({
			//	std::addressof(t), // address of object
			//	[](const void* x) { static_cast<const T*>(x)->~T(); }
			//});                    // dtor to invoke
		}

		~Vk_Rasterizer_IM(){
			VK4::Vk_Logger::Log(typeid(this), GlobalCasters::castDestructorTitle("Destroy Vk_Rasterizer_IM"));

			destroySyncResources();
			freeCommandBuffers(_device, _device->vk_renderingCommandPool(), _commandBuffer);

			_framebuffer.reset();
			_swapchain.reset();
			_renderpass.reset();
			_surfaceConfig = Vk_SurfaceConfig{ .surface=nullptr, .viewportId=0 };
		}

		void vk_update(const uint32_t imageIndex, const UniformBufferType_RendererMat4& mvp) {
			_pv->vk_update(imageIndex, static_cast<const void*>(&mvp));
		}

		// void vk_assignSurface(const Vk_SurfaceConfig& config) {
		// 	if(config.surface == nullptr) {
		// 		Vk_Logger::RuntimeError(typeid(this), "Can't assign nullptr as surface! Use vk_removeSurface() for that!");
		// 	}
		// 	if(_surfaceConfig.surface != nullptr) {
		// 		Vk_Logger::Warn(typeid(this), "Assigning surface to renderer that already has a surface assigned to it!");
		// 	}

		// 	_surfaceConfig = config;

			
		// }

		// virtual void vk_removeSurface() {
			
		// }

	protected:
	private:
		std::unique_ptr<Vk_RenderPass_IM> _renderpass;
		std::unique_ptr<Vk_Swapchain_IM> _swapchain;
		std::unique_ptr<Vk_Framebuffer_IM> _framebuffer;
		std::vector<VkCommandBuffer> _commandBuffer;
		// synchronization parts
		// A semaphore is used to add order between queue operations.
		std::vector<VkSemaphore> _imageAvailableSemaphores;
		std::vector<VkSemaphore> _renderFinishedSemaphores;
		std::vector<VkFence> _inFlightFences;

// ############################################################################################################
//   ███████ ██████        █         ██████  ███████  █████  ███  █████  ███████ ██████  ███ ███████  █████    
//   █     █ █     █       █         █     █ █       █     █  █  █     █    █    █     █  █  █       █     █   
//   █     █ █     █       █         █     █ █       █        █  █          █    █     █  █  █       █         
//   █     █ ██████        █         ██████  █████   █  ████  █   █████     █    ██████   █  █████    █████    
//   █     █ █     █ █     █         █   █   █       █     █  █        █    █    █   █    █  █             █   
//   █     █ █     █ █     █         █    █  █       █     █  █  █     █    █    █    █   █  █       █     █   
//   ███████ ██████   █████          █     █ ███████  █████  ███  █████     █    █     █ ███ ███████  █████    
// ############################################################################################################

		void recordCommandBuffers(bool resize=false) {
			// auto lock = AcquireGlobalWriteLock("vk_viewer[_recordCommandBuffers]");
			// _rebuildBeforeNextFrame = std::vector<bool>(_commandBuffer.size(), false);
			for(size_t i=0; i<_commandBuffer.size(); ++i){
				recordCommandBuffer(static_cast<int>(i), resize);
			}
		}

		void recordCommandBuffer(int index, bool resize=false) {

			VkExtent2D extent = _device->vk_swapchainSupportActiveDevice(_surfaceConfig).capabilities.currentExtent;
			auto& scFrameBuffers = *_framebuffer->vk_frameBuffers();
			// _rebuildBeforeNextFrame.at(index) = false;

			// technically not visible anymore => just set to black
			VkClearColorValue _clearValue;
			_clearValue.float32[0] = 0.0f;
			_clearValue.float32[1] = 0.0f;
			_clearValue.float32[2] = 0.0f;
			_clearValue.float32[3] = 1.0f;

			VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

			Vk_CheckVkResult(typeid(this), vkBeginCommandBuffer(_commandBuffer[index], &beginInfo), "Failed to begin recording command buffer!");

			VkRenderPassBeginInfo renderPassInfo{};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = _renderpass->vk_renderPass();
			renderPassInfo.framebuffer = scFrameBuffers[index];

			std::array<VkClearValue, 2> clearValues{};
			clearValues[0].depthStencil = { 1.0f, 0 };
			clearValues[1].color = _clearValue;
			renderPassInfo.clearValueCount = 2;
			renderPassInfo.pClearValues = clearValues.data();
			renderPassInfo.renderArea.offset = { 0,0 };
			renderPassInfo.renderArea.extent = extent;

			vkCmdBeginRenderPass(_commandBuffer[index], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

			Vk_Viewport viewport {
				.x=0,
				.y=0,
				.width=extent.width,
				.height=extent.height,
				.clearColor=Vk_RGBColor{}
			};
			//

			// if (!resize) {
			// 	for (auto& c : _cameras) {
			// 		c.second->vk_renderer()->vk_build(c.second->vk_viewport(), index, _commandBuffer);
			// 	}
			// }
			// else {
			// 	auto originalExtent = _surface->vk_canvasOriginalSize();
			// 	for (auto& c : _cameras) {
			// 		float ow = static_cast<float>(originalExtent.width);
			// 		float oh = static_cast<float>(originalExtent.height);

			// 		float x = static_cast<float>(c.second->vk_originalX()) / ow;
			// 		float y = static_cast<float>(c.second->vk_originalY()) / oh;

			// 		float w = static_cast<float>(c.second->vk_originalWidth()) / ow;
			// 		float h = static_cast<float>(c.second->vk_originalHeight()) / oh;

			// 		c.second->vk_viewport(
			// 			Vk_Viewport {
			// 				.x=static_cast<int32_t>(std::roundf(x * extent.width)),
			// 				.y=static_cast<int32_t>(std::roundf(y * extent.height)),
			// 				.width=static_cast<uint32_t>(std::roundf(w * extent.width)),
			// 				.height=static_cast<uint32_t>(std::roundf(h * extent.height))
			// 			}
			// 		);

			// 		c.second->vk_calculateTransform();
					
			// 		c.second->vk_renderer()->vk_resize(
			// 			c.second->vk_viewport(),
			// 			I_Renderer::Vk_PipelineAuxilliaries{
			// 						.surface = _surface.get(),
			// 						.renderpass = _renderpass.get(),
			// 						.swapchain = _swapchain.get(),
			// 						.framebuffer = _framebuffer.get()
			// 					},
			// 			index,
			// 			_commandBuffer
			// 		);
			// 	}
			// }

			vkCmdEndRenderPass(_commandBuffer[index]);
			Vk_CheckVkResult(typeid(this), vkEndCommandBuffer(_commandBuffer[index]), "Failed to record command buffer!");
		}

		void destroySyncResources() {
			VkDevice lDev = _device->vk_lDev();
			const auto& caps = _device->vk_swapchainSupportActiveDevice(_surface.get());
			int nFramesInFlight = caps.nFramesInFlight;
			
			for (int i = 0; i < nFramesInFlight; i++) {
				vkDestroySemaphore(lDev, _renderFinishedSemaphores[i], nullptr);
				vkDestroySemaphore(lDev, _imageAvailableSemaphores[i], nullptr);
				vkDestroyFence(lDev, _inFlightFences[i], nullptr);
			}

			_imageAvailableSemaphores.clear();
			_renderFinishedSemaphores.clear();
			_inFlightFences.clear();
		}

		void createSyncResources() {
			// create synchronization resources
			VkDevice lDev = _device->vk_lDev();
			const auto& caps = _device->vk_swapchainSupportActiveDevice(_surface.get());
			int nFramesInFlight = caps.nFramesInFlight;

			_imageAvailableSemaphores.resize(nFramesInFlight);
			_renderFinishedSemaphores.resize(nFramesInFlight);
			_inFlightFences.resize(nFramesInFlight);

			VkSemaphoreCreateInfo semaphoreInfo{};
			semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

			VkFenceCreateInfo fenceInfo{};
			fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
			// we must create the fences in the signaled state to ensure that on the first call to drawFrame
			// vkWaitForFences won't wait indefinitely
			fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

			for (int i = 0; i < nFramesInFlight; i++) {
				Vk_CheckVkResult(typeid(this), vkCreateSemaphore(lDev, &semaphoreInfo, nullptr, &_imageAvailableSemaphores[i]), "Failed to create image available semaphore for a frame!");
				Vk_CheckVkResult(typeid(this), vkCreateSemaphore(lDev, &semaphoreInfo, nullptr, &_renderFinishedSemaphores[i]), "Failed to create render finished semaphore");
				Vk_CheckVkResult(typeid(this), vkCreateFence(lDev, &fenceInfo, nullptr, &_inFlightFences[i]), "Failed to create in flight fences");
			}
		}

		void freeCommandBuffers(const Vk_Device* device, VkCommandPool commandPool, /* InOut */ std::vector<VkCommandBuffer>& commandBuffer){
			vkFreeCommandBuffers(
				device->vk_lDev(),
				commandPool,
				static_cast<uint32_t>(commandBuffer.size()),
				commandBuffer.data()
			);
		}

		void allocateCommandBuffers(
			const Vk_Device* device, 
			const Vk_SurfaceConfig& surfaceConfig, 
			VkCommandPool commandPool, 
			/* InOut */ std::vector<VkCommandBuffer>& commandBuffer
		) {
			const auto& caps = device->vk_swapchainSupportActiveDevice(surfaceConfig);
			commandBuffer.resize(caps.nFramesInFlight);
			VkCommandBufferAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			allocInfo.commandPool = commandPool;
			allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffer.size());

			Vk_CheckVkResult(typeid(this), vkAllocateCommandBuffers(
				device->vk_lDev(),
				&allocInfo,
				commandBuffer.data()),
				"Failed to allocate command buffers!"
			);
		}

		void createGraphicsPipeline(const std::string& name, const std::string& pIdentifier, const Topology topology, const CullMode cullMode, const RenderType renderType) {
			// VkDevice lDev = _device->vk_lDev();

			if (name.compare(S_Dot_P_C::Identifier) == 0) {
				auto pConf = S_Dot_P_C::getPipelineConfig(this, /*topology,*/ cullMode/*, renderType*/);
				_pipelines.insert({
						pIdentifier,
						std::make_unique<Vk_GraphicsPipeline_IM>(_viewportId, _device, _pipelineAuxilliaries.surface, pConf)
				});
			}
			else if(name.compare(S_Line_P_C::Identifier) == 0){
				auto pConf = S_Line_P_C::getPipelineConfig(this, /*topology,*/ cullMode/*, renderType*/);
				_pipelines.insert({
						pIdentifier,
						std::make_unique<Vk_GraphicsPipeline_IM>(_viewportId, _device, _pipelineAuxilliaries.surface, pConf)
				});
			}
			else if(name.compare(S_Mesh_P_C::Identifier) == 0){
				auto pConf = S_Mesh_P_C::getPipelineConfig(this, /*topology,*/ cullMode, renderType);
				_pipelines.insert({
						pIdentifier,
						std::make_unique<Vk_GraphicsPipeline_IM>(_viewportId, _device, _pipelineAuxilliaries.surface, pConf)
				});
			}
			else
				VK4::Vk_Logger::RuntimeError(typeid(this), "[createGraphicsPipeline] Object Type: {0} is not supported", name);
		}

		void createDescriptorSetLayout(const std::string& name) {
			VkDevice lDev = _device->vk_lDev();

			if (name.compare(S_Dot_P_C::Identifier) == 0) {
				_descriptorSetLayout[S_Dot_P_C::Identifier] = S_Dot_P_C::createDescriptorSetLayout(lDev);
			}
			else if (name.compare(S_Line_P_C::Identifier) == 0) {
				_descriptorSetLayout[S_Line_P_C::Identifier] = S_Line_P_C::createDescriptorSetLayout(lDev);
			}
			else if (name.compare(S_Mesh_P_C::Identifier) == 0) {
				_descriptorSetLayout[S_Mesh_P_C::Identifier] = S_Mesh_P_C::createDescriptorSetLayout(lDev);
			}
			else
				VK4::Vk_Logger::RuntimeError(typeid(this), "[createDescriptorSetLayout] Object Type: {0} is not supported", name);
		}

		void createDescriptorPool(const std::string& name) {
			VkDevice lDev = _device->vk_lDev();

			if (name.compare(S_Dot_P_C::Identifier) == 0) {
				_descriptorPools[S_Dot_P_C::Identifier].push_back(
					DescriptorPool{ .
					remaining = _freshPoolSize,
					.pool = S_Dot_P_C::createDescriptorPool(lDev, _freshPoolSize)
					});
			}
			else if (name.compare(S_Line_P_C::Identifier) == 0) {
				_descriptorPools[S_Line_P_C::Identifier].push_back(
					DescriptorPool{ .
					remaining = _freshPoolSize,
					.pool = S_Line_P_C::createDescriptorPool(lDev, _freshPoolSize)
					});
			}
			else if (name.compare(S_Mesh_P_C::Identifier) == 0) {
				_descriptorPools[S_Mesh_P_C::Identifier].push_back(
					DescriptorPool{ .
					remaining = _freshPoolSize,
					.pool = S_Mesh_P_C::createDescriptorPool(lDev, _freshPoolSize)
					});
			}
			else
				VK4::Vk_Logger::RuntimeError(typeid(this), "[createDescriptorPool] Object Type: {0} is not supported", name);
		}

		std::vector<VkDescriptorSet> createDescriptorSets(const std::string& name, VkDescriptorPool pool, int count) {
			VkDevice lDev = _device->vk_lDev();
			if (_descriptorSetLayout.find(name) == _descriptorSetLayout.end()) {
				createDescriptorSetLayout(name);
			}

			if (name.compare(S_Dot_P_C::Identifier) == 0) {
				return S_Dot_P_C::createDescriptorSets(lDev, _descriptorSetLayout.at(name), pool, count);
			}
			else if (name.compare(S_Line_P_C::Identifier) == 0) {
				return S_Line_P_C::createDescriptorSets(lDev, _descriptorSetLayout.at(name), pool, count);
			}
			else if (name.compare(S_Mesh_P_C::Identifier) == 0) {
				return S_Mesh_P_C::createDescriptorSets(lDev, _descriptorSetLayout.at(name), pool, count);
			}

			VK4::Vk_Logger::RuntimeError(typeid(this), "[createDescriptorSets] Object Type: {0} is not supported", name);
			return {};
		}


// ############################################################################################################
//   ██████  ███████ █     █ ██████           █████  ███████ █     █ █     █    █    █     █ ██████   █████    
//   █     █ █       ██    █ █     █         █     █ █     █ ██   ██ ██   ██   █ █   ██    █ █     █ █     █   
//   █     █ █       █ █   █ █     █         █       █     █ █ █ █ █ █ █ █ █  █   █  █ █   █ █     █ █         
//   ██████  █████   █  █  █ █     █         █       █     █ █  █  █ █  █  █ █     █ █  █  █ █     █  █████    
//   █   █   █       █   █ █ █     █         █       █     █ █     █ █     █ ███████ █   █ █ █     █       █   
//   █    █  █       █    ██ █     █         █     █ █     █ █     █ █     █ █     █ █    ██ █     █ █     █   
//   █     █ ███████ █     █ ██████           █████  ███████ █     █ █     █ █     █ █     █ ██████   █████    
// ############################################################################################################
		void recordRenderingCommand(const Vk_Viewport& viewport, int index, VkCommandBuffer commandBuffer){
			const auto& caps = _device->vk_swapchainSupportActiveDevice(_surfaceConfig);

			Vk_BindingProperties props{
				.capabilities = caps,
				.viewportId = _surfaceConfig.viewportId
			};
			props.uniformBuffers = std::unordered_map<int, Vk_AbstractUniformBuffer*>{ {0, _pv.get()} };

			VkClearColorValue clearValue;
			clearValue.float32[0] = viewport.clearColor.r;
			clearValue.float32[1] = viewport.clearColor.g;
			clearValue.float32[2] = viewport.clearColor.b;
			clearValue.float32[3] = 1.0f;

			VkClearAttachment clearAttachments[2] = {};
			clearAttachments[0].aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			clearAttachments[0].clearValue.color = clearValue;
			clearAttachments[0].colorAttachment = 0;
			clearAttachments[1].aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
			clearAttachments[1].clearValue.depthStencil = { 1.0f, 0 };

			int32_t offsetx = viewport.x;
			int32_t offsety = viewport.y;
			uint32_t subWidth = viewport.width;
			uint32_t subHeight = viewport.height;

			VkClearRect clearRect = {};
			clearRect.layerCount = 1;
			clearRect.rect.offset = { offsetx, offsety };
			clearRect.rect.extent = { subWidth, subHeight };

			// VkExtent2D viewportExtent = _device->vk_swapchainSupportActiveDevice(_pipelineAuxilliaries.surface).capabilities.currentExtent;

			props.frameInFlightIndex = index;
			props.commandBuffer = commandBuffer;

			// https://stackoverflow.com/questions/42501912/can-someone-help-me-understand-viewport-scissor-renderarea-framebuffer-size
			// => "The viewport specifies how the normalized device coordinates are transformed into the pixel coordinates of the framebuffer."
			VkViewport vport{
				.x = static_cast<float>(offsetx),
				.y = static_cast<float>(offsety),
				.width = static_cast<float>(subWidth),
				.height = static_cast<float>(subHeight),
				.minDepth = 0.0f,
				.maxDepth = 1.0f
			};

			// => "Scissor is the area where you can render, this is similar to viewport in that regard but changing the scissor rectangle doesn't affect the coordinates."
			VkRect2D scissor{
				.offset = VkOffset2D {
					.x = offsetx,
					.y = offsety
				},
				.extent = VkExtent2D {
					.width = static_cast<uint32_t>(subWidth),
					.height = static_cast<uint32_t>(subHeight)
				},
			};

			vkCmdClearAttachments(commandBuffer, 2, clearAttachments, 1, &clearRect);
			// ##################################################################################
			vkCmdSetViewport(commandBuffer, 0, 1, &vport);
			vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
			// ##################################################################################

			for (auto& ren : _renderables) {
				auto& obj = ren.second;
				obj->vk_00_bindPipeline(props);
				obj->vk_10_bindLineWidth(props);
				obj->vk_20_bindDataBuffer(props);
				obj->vk_30_bindPushConstants(props);
				obj->vk_40_bindUniformBuffers(props);
				obj->vk_50_bindDescriptorSets(props);
				obj->vk_99_bindFinalize(props);
			}
		}

		void recordRenderingCommands(const Vk_Viewport& viewport, const std::vector<VkCommandBuffer>& commandBuffer) {
			const auto& caps = _device->vk_swapchainSupportActiveDevice(_pipelineAuxilliaries.surface);
			int nFramesInFlight = caps.nFramesInFlight;
			for (int i = 0; i < nFramesInFlight; ++i) {
				recordRenderingCommand(viewport, i, commandBuffer[i]);
			}
		}
	};
}