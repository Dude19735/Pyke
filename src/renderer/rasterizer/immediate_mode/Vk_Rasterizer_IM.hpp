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
			const Vk_RGBColor clearColor,
			const float clearAlpha,
			const int freshPoolSize,
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
			_inFlightFences({}),
			_clearColor(clearColor),
			_clearAlpha(clearAlpha),
			_currentFrame(0)
		{
			VK4::Vk_Logger::Log(typeid(this), GlobalCasters::castConstructorTitle("Create Vk_Rasterizer_IM"));
			allocateCommandBuffers(_device, _surfaceConfig, _device->vk_renderingCommandPool(), _commandBuffer);
			createResolutionRelatedResources();
			
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

		void vk_setClearValue(const Vk_RGBColor& color, float alpha) {
			_clearColor = color;
			_clearAlpha = alpha;
		}

		bool vk_waitFinish() {
			/**
			 * NOTE: _currentFrame is really the current frame and not the last one. No _currentFrame+1 necessary
			 */
			// We have to wait for the current fence to be signaled to make sure that
			auto lDev = _device->vk_lDev();
			VkResult res = vkWaitForFences(lDev, 1, &_inFlightFences[_currentFrame], VK_TRUE, GLOBAL_FENCE_TIMEOUT);
			if(res == VK_TIMEOUT){
				Vk_Logger::Warn(typeid(this), "drawFrame timeout for frame index [{0}]", _currentFrame);
				return false;
			}
			else if (res != VK_SUCCESS) {
				// TODO: this one may be fatal => potentially kill the renderer
				Vk_Logger::Warn(typeid(this), "Waiting for fences had catastrphic result!");
				return false;
			}
			return true;
		}

		bool vk_nextImage(const UniformBufferType_RendererMat4& mvp) {
			auto lDev = _device->vk_lDev();

			/**
			 * TODO: figure out if rebuild is necessary
			 */
			recordCommandBuffer(_currentFrame);

			// acquire next image enqueue job
			// NOTE: imageIndex != _currentFrame is possible => watch out for that!
			uint32_t imageIndex;
			VkResult result = vkAcquireNextImageKHR(
				lDev, _swapchain->vk_swapchain(), GLOBAL_FENCE_TIMEOUT,
				_imageAvailableSemaphores[_currentFrame], // this one is signaled once the presentation queue is done with this image
				VK_NULL_HANDLE, &imageIndex
			);

			/**
			 * TODO: put error handling inside some Vk_RendererLib method
			 */
			if(Vk_RendererLib::checkSubmitResult(typeid(this), "vkAcquireNextImageKHR", result) != Vk_SubmitResult::Ok){
				resetViewer();
				return false;
			}

			// update the uniform buffer
			_pv->vk_update(imageIndex, static_cast<const void*>(&mvp));

			vkResetFences(lDev, 1, &_inFlightFences[_currentFrame]);

			/**
			 * TODO: put this into some function
			 */
			VkSubmitInfo submitInfo{};
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

			VkSemaphore waitSemaphores[] = { _imageAvailableSemaphores[currentFrame] };
			VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
			submitInfo.waitSemaphoreCount = 1;
			submitInfo.pWaitSemaphores = waitSemaphores;
			submitInfo.pWaitDstStageMask = waitStages;
			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &_commandBuffer[currentFrame];

			VkSemaphore signalSemaphores[] = { _renderFinishedSemaphores[currentFrame] };
			submitInfo.signalSemaphoreCount = 1;
			submitInfo.pSignalSemaphores = signalSemaphores;
			
			/**
			 * TODO: eliminate this ThreadSafe submit
			 */
			Vk_CheckVkResult(typeid(this), 
				Vk_ThreadSafe::Vk_ThreadSafe_QueueSubmit(
					_device->vk_graphicsQueue(), 1, &submitInfo, _inFlightFences[_currentFrame]
				),
				"Failed to submit draw command buffer!"
			);

			/**
			 * TODO: put this into some function too
			 */
			VkPresentInfoKHR presentInfo{};
			presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
			presentInfo.waitSemaphoreCount = 1;
			presentInfo.pWaitSemaphores = signalSemaphores;

			VkSwapchainKHR swapChains[] = { _swapchain->vk_swapchain() };
			presentInfo.swapchainCount = 1;
			presentInfo.pSwapchains = swapChains;
			presentInfo.pImageIndices = &imageIndex;

			result = vkQueuePresentKHR(_device->vk_presentationQueue(), &presentInfo);	

			/**
			 * TODO: outsource this into some Vk_RendererLib class
			 */
			if(Vk_RendererLib::checkSubmitResult(typeid(this), "vkQueuePresentKHR", result) != Vk_SubmitResult::Ok){
				resetViewer();
				return false;
			}

			_currentFrame++;
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

		Vk_RGBColor _clearColor;
		float _clearAlpha;
		int _currentFrame;

// ############################################################################################################
//   ███████ ██████        █         ██████  ███████  █████  ███  █████  ███████ ██████  ███ ███████  █████    
//   █     █ █     █       █         █     █ █       █     █  █  █     █    █    █     █  █  █       █     █   
//   █     █ █     █       █         █     █ █       █        █  █          █    █     █  █  █       █         
//   █     █ ██████        █         ██████  █████   █  ████  █   █████     █    ██████   █  █████    █████    
//   █     █ █     █ █     █         █   █   █       █     █  █        █    █    █   █    █  █             █   
//   █     █ █     █ █     █         █    █  █       █     █  █  █     █    █    █    █   █  █       █     █   
//   ███████ ██████   █████          █     █ ███████  █████  ███  █████     █    █     █ ███ ███████  █████    
// ############################################################################################################

		/**
		 * TODO: abstract this away with some generic registration mechanism
		 */
		void createGraphicsPipeline(const std::string& name, const std::string& pIdentifier, const Topology topology, const CullMode cullMode, const RenderType renderType) {
			// VkDevice lDev = _device->vk_lDev();

			if (name.compare(S_Dot_P_C::Identifier) == 0) {
				auto pConf = S_Dot_P_C::getPipelineConfig(this, /*topology,*/ cullMode/*, renderType*/);
				_pipelines.insert({
						pIdentifier,
						std::make_unique<Vk_GraphicsPipeline_IM>(_surfaceConfig.viewportId, _device, _pipelineAuxilliaries.surface, pConf)
				});
			}
			else if(name.compare(S_Line_P_C::Identifier) == 0){
				auto pConf = S_Line_P_C::getPipelineConfig(this, /*topology,*/ cullMode/*, renderType*/);
				_pipelines.insert({
						pIdentifier,
						std::make_unique<Vk_GraphicsPipeline_IM>(_surfaceConfig.viewportId, _device, _pipelineAuxilliaries.surface, pConf)
				});
			}
			else if(name.compare(S_Mesh_P_C::Identifier) == 0){
				auto pConf = S_Mesh_P_C::getPipelineConfig(this, /*topology,*/ cullMode, renderType);
				_pipelines.insert({
						pIdentifier,
						std::make_unique<Vk_GraphicsPipeline_IM>(_surfaceConfig.viewportId, _device, _pipelineAuxilliaries.surface, pConf)
				});
			}
			else
				VK4::Vk_Logger::RuntimeError(typeid(this), "[createGraphicsPipeline] Object Type: {0} is not supported", name);
		}

		/**
		 * TODO: abstract this away with some generic registration mechanism
		 */
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

		/**
		 * TODO: abstract this away with some generic registration mechanism
		 */
		void createDescriptorPool(const std::string& name) {
			VkDevice lDev = _device->vk_lDev();

			if (name.compare(S_Dot_P_C::Identifier) == 0) {
				_descriptorPools[S_Dot_P_C::Identifier].push_back(
					DescriptorPool{ 
						.remaining = _freshPoolSize,
						.pool = S_Dot_P_C::createDescriptorPool(lDev, _freshPoolSize)
					});
			}
			else if (name.compare(S_Line_P_C::Identifier) == 0) {
				_descriptorPools[S_Line_P_C::Identifier].push_back(
					DescriptorPool{ 
						.remaining = _freshPoolSize,
						.pool = S_Line_P_C::createDescriptorPool(lDev, _freshPoolSize)
					});
			}
			else if (name.compare(S_Mesh_P_C::Identifier) == 0) {
				_descriptorPools[S_Mesh_P_C::Identifier].push_back(
					DescriptorPool{ 
						.remaining = _freshPoolSize,
						.pool = S_Mesh_P_C::createDescriptorPool(lDev, _freshPoolSize)
					});
			}
			else
				VK4::Vk_Logger::RuntimeError(typeid(this), "[createDescriptorPool] Object Type: {0} is not supported", name);
		}

		/**
		 * TODO: abstract this away with some generic registration mechanism
		 */
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
		void resetViewer() {
			/**
			 * TODO: add a local lock here
			 */
			// auto lock = AcquireGlobalWriteLock("vk_viewer[_onWindowAction(resetViewer)]");
			// _onResize = true;

			/**
			 * TODO: only wait for the queues assigned to this one to be idle
			 */
			// Vk_ThreadSafe::Vk_ThreadSafe_DeviceWaitIdle(_device->vk_lDev());

			destroySyncResources(); // must be done before swapchainSupport is invalidated to keep correct nFramesInFlight!
			_device->vk_invalidateSwapchainSupport();
			createResolutionRelatedResources();
			// _device->bridge.setCurrentFrameTo(0);
			// _onResize = false;
		}

		void createResolutionRelatedResources() {
			_device->vk_swapchainSupportActiveDevice(_surface.get());

			_renderpass = std::make_unique<Vk_RenderPass_IM>(_device, _surface.get());
			_swapchain = std::make_unique<Vk_Swapchain_IM>(_device, _surface.get());
			_framebuffer = std::make_unique<Vk_Framebuffer_IM>(_device, _surface.get(), _swapchain.get(), _renderpass.get());
			createSyncResources(); // must be done after swapchain creation to get correct nFramesInFlight

			recordCommandBuffers(true);
			_currentFrame = 0;
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

		void recordCommandBuffers() {
			for(size_t i=0; i<_commandBuffer.size(); ++i){
				recordCommandBuffer(static_cast<int>(i));
			}
		}

		void recordCommandBuffer(int index) {
			// Get the current size of the viewport
			VkExtent2D extent = _device->vk_swapchainSupportActiveDevice(_surfaceConfig).capabilities.currentExtent;
			auto& scFrameBuffers = *_framebuffer->vk_frameBuffers();
			// _rebuildBeforeNextFrame.at(index) = false;

			// technically not visible anymore => just set to black
			VkClearColorValue clearValue;
			clearValue.float32[0] = _clearColor.r;
			clearValue.float32[1] = _clearColor.g;
			clearValue.float32[2] = _clearColor.b;
			clearValue.float32[3] = _clearAlpha;

			VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

			Vk_CheckVkResult(typeid(this), vkBeginCommandBuffer(_commandBuffer[index], &beginInfo), "Failed to begin recording command buffer!");

			VkRenderPassBeginInfo renderPassInfo{};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = _renderpass->vk_renderPass();
			renderPassInfo.framebuffer = scFrameBuffers[index];

			std::array<VkClearValue, 2> clearValues{};
			clearValues[0].depthStencil = { 1.0f, 0 };
			clearValues[1].color = clearValue;

			renderPassInfo.clearValueCount = 2;
			renderPassInfo.pClearValues = clearValues.data();
			renderPassInfo.renderArea.offset = { 0,0 };
			renderPassInfo.renderArea.extent = extent;

			vkCmdBeginRenderPass(_commandBuffer[index], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

			
			const auto& caps = _device->vk_swapchainSupportActiveDevice(_surfaceConfig);

			Vk_BindingProperties props{
				.capabilities = caps,
				.viewportId = _surfaceConfig.viewportId
			};
			props.uniformBuffers = std::unordered_map<int, Vk_AbstractUniformBuffer*>{ {0, _pv.get()} };

			VkClearColorValue clearValue;
			clearValue.float32[0] = viewport.r;
			clearValue.float32[1] = viewport.g;
			clearValue.float32[2] = viewport.b;
			clearValue.float32[3] = 1.0f;

			VkClearAttachment clearAttachments[2] = {};
			clearAttachments[0].aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			clearAttachments[0].clearValue.color = clearValue;
			clearAttachments[0].colorAttachment = 0;
			clearAttachments[1].aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
			clearAttachments[1].clearValue.depthStencil = { 1.0f, 0 };

			VkClearRect clearRect = {};
			clearRect.layerCount = 1;
			clearRect.rect.offset = { 0, 0 };
			clearRect.rect.extent = { extent.width, extent.height };

			props.frameInFlightIndex = index;
			props.commandBuffer = _commandBuffer[index];

			vkCmdClearAttachments(_commandBuffer[index], 2, clearAttachments, 1, &clearRect);

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

			vkCmdEndRenderPass(_commandBuffer[index]);
			Vk_CheckVkResult(typeid(this), vkEndCommandBuffer(_commandBuffer[index]), "Failed to record command buffer!");
		}
	};
}