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
			int camId,
			Vk_Device* const device,
			I_Renderer::Vk_PipelineAuxilliaries auxilliaries,
			int freshPoolSize,
			UniformBufferType_RendererMat4 mvpInit
		)
			:
			I_Renderer(
				//I_Renderer::Caster{
				//	.p = std::addressof(*this),
				//	.cast = [](void* x) { static_cast<Vk_Rasterizer_IM*>(x)->Destroy(); }
				//},
				camId, device, auxilliaries,
				freshPoolSize, mvpInit
			)
		{
			VK4::Vk_Logger::Log(typeid(this), GlobalCasters::castConstructorTitle("Create Vk_Rasterizer_IM"));

			//dtors.push_back({
			//	std::addressof(t), // address of object
			//	[](const void* x) { static_cast<const T*>(x)->~T(); }
			//});                    // dtor to invoke
		}

		~Vk_Rasterizer_IM(){
			VK4::Vk_Logger::Log(typeid(this), GlobalCasters::castDestructorTitle("Destroy Vk_Rasterizer_IM"));
		}

		void vk_update(const uint32_t imageIndex, const UniformBufferType_RendererMat4& mvp) {
			_pv->vk_update(imageIndex, static_cast<const void*>(&mvp));
		}

	protected:
	private:

// ############################################################################################################
//   ███████ ██████        █         ██████  ███████  █████  ███  █████  ███████ ██████  ███ ███████  █████    
//   █     █ █     █       █         █     █ █       █     █  █  █     █    █    █     █  █  █       █     █   
//   █     █ █     █       █         █     █ █       █        █  █          █    █     █  █  █       █         
//   █     █ ██████        █         ██████  █████   █  ████  █   █████     █    ██████   █  █████    █████    
//   █     █ █     █ █     █         █   █   █       █     █  █        █    █    █   █    █  █             █   
//   █     █ █     █ █     █         █    █  █       █     █  █  █     █    █    █    █   █  █       █     █   
//   ███████ ██████   █████          █     █ ███████  █████  ███  █████     █    █     █ ███ ███████  █████    
// ############################################################################################################
		void createGraphicsPipeline(const std::string& name, const std::string& pIdentifier, const Topology topology, const CullMode cullMode, const RenderType renderType) {
			// VkDevice lDev = _device->vk_lDev();

			if (name.compare(S_Dot_P_C::Identifier) == 0) {
				auto pConf = S_Dot_P_C::getPipelineConfig(this, /*topology,*/ cullMode/*, renderType*/);
				_pipelines.insert({
						pIdentifier,
						std::make_unique<Vk_GraphicsPipeline_IM>(_camId, _device, _pipelineAuxilliaries.surface, pConf)
				});
			}
			else if(name.compare(S_Line_P_C::Identifier) == 0){
				auto pConf = S_Line_P_C::getPipelineConfig(this, /*topology,*/ cullMode/*, renderType*/);
				_pipelines.insert({
						pIdentifier,
						std::make_unique<Vk_GraphicsPipeline_IM>(_camId, _device, _pipelineAuxilliaries.surface, pConf)
				});
			}
			else if(name.compare(S_Mesh_P_C::Identifier) == 0){
				auto pConf = S_Mesh_P_C::getPipelineConfig(this, /*topology,*/ cullMode, renderType);
				_pipelines.insert({
						pIdentifier,
						std::make_unique<Vk_GraphicsPipeline_IM>(_camId, _device, _pipelineAuxilliaries.surface, pConf)
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
			const auto& caps = _device->vk_swapchainSupportActiveDevice(_pipelineAuxilliaries.surface);

			Vk_BindingProperties props{
				.capabilities = caps,
				.camId = _camId
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