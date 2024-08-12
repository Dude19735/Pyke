# pragma once

// #include <vulkan/vulkan.h>
#include "../Defines.h"

namespace VK4 {
	class Vk_Initializers {
	public:
		static VkPipelineShaderStageCreateInfo init_PipelineShaderStageCreateInfo(
			/*VkPipelineStageFlagBits shaderStage,*/
			VkShaderStageFlagBits shaderStage,
			VkShaderModule module
		) {
			VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
			vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			vertShaderStageInfo.stage = shaderStage;
			vertShaderStageInfo.module = module;
			vertShaderStageInfo.pName = "main";

			return vertShaderStageInfo;
		}
	};
}