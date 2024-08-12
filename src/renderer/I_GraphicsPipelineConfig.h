#pragma once

namespace VK4 {
	/*
	* Maybe having just one of these could really be worth it...
	* Just have one that can be extended...
	*/

	struct Vk_Config_GraphicsPipeline_IM {
	public:
		const VkPrimitiveTopology topology;
		const uint32_t sizeofPushConstants;
		const CullMode cullMode;
		const RenderType renderType;
		const Vk_Shader* vertexShader;
		const Vk_Shader* fragmentShader;
		const I_RenderPass* renderPass;
		const VkDescriptorSetLayout descriptorSetLayout;
		const std::vector<VkVertexInputBindingDescription> bindingDescription;
		const std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
	};

	struct Vk_Config_GraphicsPipeline_Dummy {
	public:
		const VkPrimitiveTopology topology;
		const uint32_t sizeofPushConstants;
		const CullMode cullMode;
		const RenderType renderType;
		const Vk_Shader* vertexShader;
		const Vk_Shader* fragmentShader;
		const I_RenderPass* renderPass;
		const VkDescriptorSetLayout descriptorSetLayout;
		const std::vector<VkVertexInputBindingDescription> bindingDescription;
		const std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
	};
}