#pragma once

// #include <vulkan/vulkan.h>
#include <string>

#include "../../../Defines.h"
// #include "../../../Vk_Logger.hpp"
#include "../../I_GraphicsPipeline.hpp"
#include "Vk_RenderPass_IM.hpp"

//#include "../../application/Vk_Application.hpp"
//#include "../../swapchains/Vk_RenderSpec.hpp"
//#include "../../descriptors/Vk_DescriptorSetLayout.hpp"
//#include "../../objects/Vk_Structures.hpp"
//#include "../../shading/shadermodules/Vk_Shader.hpp"

namespace VK4 {

	class Vk_GraphicsPipeline_IM : public I_GraphicsPipeline {
	public:

		Vk_GraphicsPipeline_IM(
			int camId,
			Vk_Device* const device,
			const Vk_Surface* surface,
			const Vk_Config_GraphicsPipeline_IM& config
		) : _surface(surface)
		{
			VK4::Vk_Logger::Log(typeid(this), GlobalCasters::castConstructorTitle("Create IM Graphics Pipeline"));

			_device = device;

			// create shader stages
			VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
			vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
			vertShaderStageInfo.module = config.vertexShader->vk_shaderModule();
			vertShaderStageInfo.pName = "main"; // entry point into shader, combine multiple shaders using different entry points

			VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
			fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
			fragShaderStageInfo.module = config.fragmentShader->vk_shaderModule();
			fragShaderStageInfo.pName = "main";

			VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

			// create vertex input info
			// describe vertex data
			// use the specially defined vertex struct from the top
			//std::vector<VkVertexInputBindingDescription> bindingDescription;
			//bindingDescription.push_back(Geometry::Vk_Structure_Vertex::getBindingDescription());
			//bindingDescription.push_back(Geometry::Vk_Structure_Color::getBindingDescription());
			//if (_shadingType != ShadingType::None) {
			//	bindingDescription.push_back(Geometry::Vk_Structure_Normal::getBindingDescription());
			//}

			//std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
			//attributeDescriptions.push_back(Geometry::Vk_Structure_Vertex::getAttributeDescriptions());
			//attributeDescriptions.push_back(Geometry::Vk_Structure_Color::getAttributeDescriptions());
			//if (_shadingType != ShadingType::None) {
			//	attributeDescriptions.push_back(Geometry::Vk_Structure_Normal::getAttributeDescriptions());
			//}

			VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
			vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
			vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(config.bindingDescription.size());
			vertexInputInfo.pVertexBindingDescriptions = config.bindingDescription.data();
			vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(config.attributeDescriptions.size());
			vertexInputInfo.pVertexAttributeDescriptions = config.attributeDescriptions.data();

			// input assembly, describe how vertex data is structured
			VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
			inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
			inputAssembly.topology = config.topology;
			inputAssembly.primitiveRestartEnable = VK_FALSE; // enable or not if vertice can be used for multiple triangles

			VkExtent2D viewp = _device->vk_swapchainSupportActiveDevice(_surface).capabilities.currentExtent;

			// define viewport, usually between 0,0 and with,height
			VkViewport viewport = {};
			viewport.x = 0.0f;
			viewport.y = 0.0f;
			viewport.width = static_cast<float>(viewp.width);
			viewport.height = static_cast<float>(viewp.height);
			viewport.minDepth = 0.0f; // stick to this
			viewport.maxDepth = 1.0f; // stick to this

			// define scissor
			VkRect2D scissor = {};
			scissor.offset = { 0, 0 };
			scissor.extent = VkExtent2D{ .width = viewp.width, .height = viewp.height };

			// combine scissor and viewport into one create info
			VkPipelineViewportStateCreateInfo viewportState = {};
			viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
			viewportState.viewportCount = 1;
			viewportState.pViewports = &viewport;
			viewportState.scissorCount = 1;
			viewportState.pScissors = &scissor;

			// configure rasterizer
			VkPipelineRasterizationStateCreateInfo rasterizer = {};
			rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
			rasterizer.depthClampEnable = VK_FALSE;
			rasterizer.rasterizerDiscardEnable = VK_FALSE; // disable the frame buffer, true => geometry never passes trough the rasterizer

			if (config.renderType == RenderType::Solid) {
				rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
			}
			else if (config.renderType == RenderType::Wireframe) {
				rasterizer.polygonMode = VK_POLYGON_MODE_LINE;
			}
			else if (config.renderType == RenderType::Point) {
				rasterizer.polygonMode = VK_POLYGON_MODE_POINT;
			}
			else {
				VK4::Vk_Logger::RuntimeError(typeid(this), std::string("RenderType: ") + RenderTypeToString(config.renderType) + " is not defined");
			}

			if (config.cullMode == CullMode::Back) {
				rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
			}
			else if (config.cullMode == CullMode::Front) {
				rasterizer.cullMode = VK_CULL_MODE_FRONT_BIT;
			}
			else if (config.cullMode == CullMode::NoCulling) {
				rasterizer.cullMode = VK_CULL_MODE_NONE;
			}
			else {
				VK4::Vk_Logger::RuntimeError(typeid(this), std::string("CullMode: ") + CullModeToString(config.cullMode) + " is not defined");
			}

			//rasterizer.polygonMode = VK_POLYGON_MODE_LINE; // renderingmode
			//rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
			// any line width other than 1.0 requires activation of wideLines GPU feature
			rasterizer.lineWidth = 1.0f; // default line width

			// determine wheather to render background or not
			//rasterizer.cullMode = VK_CULL_MODE_BACK_BIT; // this means render foreground
			//rasterizer.cullMode = VK_CULL_MODE_FRONT_AND_BACK; // this means render nothing
			//rasterizer.cullMode = VK_CULL_MODE_NONE; // this means render everything

			rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
			//rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;

			// determine depth bias configuration
			rasterizer.depthBiasEnable = VK_FALSE;
			//rasterizer.depthBiasConstantFactor = 0.0f;
			//rasterizer.depthBiasClamp = 0.0f;
			//rasterizer.depthBiasClamp = 0.0f;
			//rasterizer.depthBiasSlopeFactor = 0.0f;

			VkPipelineMultisampleStateCreateInfo multisampling = {};
			multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
			multisampling.sampleShadingEnable = VK_FALSE;
			multisampling.rasterizationSamples = _device->vk_sampleCount();

			// color blending, how to mix old and new color in the framebuffer (individually for every framebuffer)
			VkPipelineColorBlendAttachmentState colorBlendAttachement = {};
			colorBlendAttachement.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
			colorBlendAttachement.blendEnable = VK_TRUE; // false: don't use this feature
			colorBlendAttachement.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
			colorBlendAttachement.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
			colorBlendAttachement.colorBlendOp = VK_BLEND_OP_ADD;
			colorBlendAttachement.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
			colorBlendAttachement.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
			colorBlendAttachement.alphaBlendOp = VK_BLEND_OP_ADD;

			// color blending info structure
			// set the same for all framebuffers
			// if both modes are disabled, the new value will simply be copied into the framebuffer
			VkPipelineColorBlendStateCreateInfo colorBlending = {};
			colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
			colorBlending.logicOpEnable = VK_FALSE;
			colorBlending.logicOp = VK_LOGIC_OP_COPY;
			colorBlending.attachmentCount = 1;
			colorBlending.pAttachments = &colorBlendAttachement;
			colorBlending.blendConstants[0] = 0.0f;
			colorBlending.blendConstants[1] = 0.0f;
			colorBlending.blendConstants[2] = 0.0f;
			colorBlending.blendConstants[3] = 0.0f;

			// a limited amount of states can be changed without recreating the pipeline
			// for example the size of the viewport, line width and blend constants
			// requires to specify the corresponding data at drawing time
			std::vector<VkDynamicState> dynamicStates{
				VK_DYNAMIC_STATE_LINE_WIDTH,
				VK_DYNAMIC_STATE_VIEWPORT,
				VK_DYNAMIC_STATE_SCISSOR
			};

			VkPipelineDynamicStateCreateInfo dynamicState = {};
			dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
			dynamicState.dynamicStateCount = static_cast<std::uint32_t>(dynamicStates.size());
			dynamicState.pDynamicStates = dynamicStates.data();

			// create push constants
			VkPushConstantRange pushConstant = {};
			pushConstant.offset = 0;
			pushConstant.size = config.sizeofPushConstants;
			pushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

			// create a pipeline layout
			// if no uniforms are required, create an empty one
			VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
			pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			pipelineLayoutInfo.setLayoutCount = 1; // set this to the amount of layouts that should be bound
			pipelineLayoutInfo.pSetLayouts = &config.descriptorSetLayout;
			pipelineLayoutInfo.pushConstantRangeCount = 1;
			pipelineLayoutInfo.pPushConstantRanges = &pushConstant;

			VK_CHECK(vkCreatePipelineLayout(
				_device->vk_lDev(),
				&pipelineLayoutInfo,
				nullptr, &_pipelineLayout),
				"Failed to create pipeline layout"
			);

			// enable depth testing in the graphics pipeline
			VkPipelineDepthStencilStateCreateInfo depthStencil = {};
			depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
			depthStencil.depthTestEnable = VK_TRUE; // enable comparison of fragments to see if they should be drawn or not
			depthStencil.depthWriteEnable = VK_TRUE; // enable writing of the depth of a new fragment to the depth buffer if it's drawn (usefull for transparent stuff)
			depthStencil.depthCompareOp = VK_COMPARE_OP_LESS; // operation used to discard or keep fragments
			depthStencil.depthBoundsTestEnable = VK_FALSE; // disable keeping fragments that fall within minDepthBounds and maxDepthBounds
			//depthStencil.minDepthBounds = 0.0f;
			//depthStencil.maxDepthBounds = 1.0f;
			depthStencil.stencilTestEnable = VK_FALSE; // disable stencil testing (if enabled, all formats for depth testing must contain a stencil component)
			//depthStencil.front = {};
			//depthStencil.back = {};

			// now create the graphics pipeline createinfo structure
			VkGraphicsPipelineCreateInfo pipelineInfo = {};
			pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
			// shader stages
			pipelineInfo.stageCount = 2;
			pipelineInfo.pStages = shaderStages;
			// all info structs for fixed function stages
			pipelineInfo.pVertexInputState = &vertexInputInfo;
			pipelineInfo.pInputAssemblyState = &inputAssembly;
			pipelineInfo.pViewportState = &viewportState;
			pipelineInfo.pRasterizationState = &rasterizer;
			pipelineInfo.pMultisampleState = &multisampling;
			pipelineInfo.pDepthStencilState = &depthStencil;
			pipelineInfo.pColorBlendState = &colorBlending;
			pipelineInfo.pDynamicState = &dynamicState;
			pipelineInfo.layout = _pipelineLayout;
			pipelineInfo.renderPass = config.renderPass->vk_renderPass(); // define render pass for this pipeline
			pipelineInfo.subpass = 0; // define index of the subpass where this pipeline will be used
			pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // possibility to create pipeline by inheriting from existing pipeline
			pipelineInfo.basePipelineIndex = -1;
			// pipelineInfo.flags = VK_PIPELINE_CREATE_DERIVATIVE_BIT; // specify this to use pipeline derivatives

			VK_CHECK(vkCreateGraphicsPipelines(
				_device->vk_lDev(),
				VK_NULL_HANDLE,
				1,
				&pipelineInfo,
				nullptr,
				&_pipeline),
				"Failed to create graphics pipeline"
			);
		}

		~Vk_GraphicsPipeline_IM() {
			VK4::Vk_Logger::Log(typeid(this), GlobalCasters::castDestructorTitle("Destroy Graphics Pipeline IM"));
		}

		const VkPipelineLayout vk_pipelineLayout() const {
			return _pipelineLayout;
		}

		const VkPipeline vk_pipeline() const {
			return _pipeline;
		}

	private:
		const Vk_Surface* _surface;
	};
}
