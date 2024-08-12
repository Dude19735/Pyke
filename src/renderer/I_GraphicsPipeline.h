#pragma once

// #include <vulkan/vulkan.h>
#include "../Defines.h"
#include "I_GraphicsPipeline.h"
#include "I_GraphicsPipelineConfig.h"
// #include "../Vk_Logger.h"
#include "../application/Vk_Device.h"

namespace VK4 {
	class I_GraphicsPipeline {
	public:
		static std::string createGraphicsPipelineIndentifier(
			const std::string& identifier, 
			Topology topology, 
			CullMode cullMode, 
			RenderType renderType
		) {
			return 
				identifier + "|" + 
				TopologyToString(topology) + "|" + 
				CullModeToString(cullMode) + "|" + 
				RenderTypeToString(renderType);
		}

		virtual ~I_GraphicsPipeline() {
			VK4::Vk_Logger::Log(typeid(this), GlobalCasters::castDestructorTitle("Destroy I Graphics Pipeline"));

			VkDevice lDev = _device->vk_lDev();
			vkDestroyPipeline(lDev, _pipeline, nullptr);
			vkDestroyPipelineLayout(lDev, _pipelineLayout, nullptr);
		}

		virtual const VkPipeline vk_pipeline() const = 0;
		virtual const VkPipelineLayout vk_pipelineLayout() const = 0;

	protected:
		Vk_Device* _device;
		VkPipelineLayout _pipelineLayout;
		VkPipeline _pipeline;
	};
}