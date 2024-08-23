#ifndef BOOST_TEST_INCLUDED
    #include "boost/test/included/unit_test.hpp"
#endif

#include <iostream>
#include <typeinfo>
#include <assert.h>
#include <chrono>

#include <math.h>
#define _USE_MATH_DEFINES

#include "../src/Defines.h"

// #include "../src/vk_renderer4/renderer/rasterizer/immediate_mode/Vk_Graphicspipeline_IM.hpp"
// #include "../src/vk_renderer4/renderer/rasterizer/immediate_mode/Vk_Rasterizer_IM.hpp"

#include "../src/Vk_ColorOp.hpp"
#include "../src/application/Vk_Instance.hpp"
#include "../src/application/Vk_Surface.hpp"
#include "../src/application/Vk_Device.hpp"
#include "../src/buffers/Vk_DataBuffer.hpp"
#include "../src/buffers/Vk_UniformBuffer.hpp"
#include "../src/objects/Vk_Shader.hpp"
#include "../src/objects/dot/S_Dot_P_C.hpp"
#include "../src/objects/line/S_Line_P_C.hpp"
#include "../src/objects/mesh/S_Mesh_P_C.hpp"
#include "../src/renderer/rasterizer/immediate_mode/Vk_Swapchain_IM.hpp"
#include "../src/renderer/rasterizer/immediate_mode/Vk_RenderPass_IM.hpp"
#include "../src/renderer/rasterizer/immediate_mode/Vk_Framebuffer_IM.hpp"
#include "../src/renderer/rasterizer/immediate_mode/Vk_Rasterizer_IM.hpp"
#include "../src/camera/Vk_GridLayout.hpp"
#include "../src/camera/Vk_ViewerSteering_ObjectCentric.hpp"
#include "../src/Vk_Function.hpp"

// #include "../src/vk_renderer4/renderer/rasterizer/immediate_mode/Vk_Graphicspipeline_IM.hpp"

#include "../src/Vk_Viewer.hpp"

#define TEST_SCENARIO_1
#include "test_data.hpp"
#undef TEST_SCENARIO_1
#include "test_utilities.hpp"

BOOST_AUTO_TEST_SUITE(RunTestVk4Components)

auto new_test = boost::unit_test::enabled();
auto all_tests = boost::unit_test::disabled();

BOOST_AUTO_TEST_CASE(Test_Swapchain, *new_test) {
	std::string name = "test_app";

	int camId = 0;
	{
		std::unique_ptr<VK4::Vk_Device> device = std::make_unique<VK4::Vk_Device>(name);
        std::unique_ptr<VK4::Vk_Surface> surface = std::make_unique<VK4::Vk_Surface>(device->vk_instance(), name, 500, 300, true);
		std::unique_ptr<VK4::Vk_Swapchain_IM> swapchain = std::make_unique<VK4::Vk_Swapchain_IM>(device.get(), surface.get());
	}
	{
		std::unique_ptr<VK4::Vk_Device> device = std::make_unique<VK4::Vk_Device>(name);
        std::unique_ptr<VK4::Vk_Surface> surface = std::make_unique<VK4::Vk_Surface>(device->vk_instance(), name, 500, 300, true);
		// try multiple times just to properly check the deconstructor
		{
			std::unique_ptr<VK4::Vk_Swapchain_IM> swapchain = std::make_unique<VK4::Vk_Swapchain_IM>(device.get(), surface.get());
		}
		{
			std::unique_ptr<VK4::Vk_Swapchain_IM> swapchain = std::make_unique<VK4::Vk_Swapchain_IM>(device.get(), surface.get());
		}
		{
			std::unique_ptr<VK4::Vk_Swapchain_IM> swapchain = std::make_unique<VK4::Vk_Swapchain_IM>(device.get(), surface.get());
		}
	}
}

BOOST_AUTO_TEST_CASE(Test_Renderpass, *new_test) {
	std::string name = "test_app";

	int camId = 0;
	{
		std::unique_ptr<VK4::Vk_Device> device = std::make_unique<VK4::Vk_Device>(name);
        std::unique_ptr<VK4::Vk_Surface> surface = std::make_unique<VK4::Vk_Surface>(device->vk_instance(), name, 500, 300, true);
		std::unique_ptr<VK4::Vk_RenderPass_IM> renderpass = std::make_unique<VK4::Vk_RenderPass_IM>(device.get(), surface.get());
	}
}

BOOST_AUTO_TEST_CASE(Test_Framebuffer, *new_test) {
	std::string name = "test_app";

	{
		int camId = 0;
		std::unique_ptr<VK4::Vk_Device> device = std::make_unique<VK4::Vk_Device>(name);
		std::unique_ptr<VK4::Vk_Surface> surface = std::make_unique<VK4::Vk_Surface>(device->vk_instance(), name, 500, 300, true);
		std::unique_ptr<VK4::Vk_RenderPass_IM> renderpass = std::make_unique<VK4::Vk_RenderPass_IM>(device.get(), surface.get());
		std::unique_ptr<VK4::Vk_Swapchain_IM> swapchain = std::make_unique<VK4::Vk_Swapchain_IM>(device.get(), surface.get());
		std::unique_ptr<VK4::Vk_Framebuffer_IM> framebuffer = std::make_unique<VK4::Vk_Framebuffer_IM>(device.get(), surface.get(), swapchain.get(), renderpass.get());
	}
}

BOOST_AUTO_TEST_CASE(Test_Graphicspipeline, *new_test) {
	std::string name = "test_app";

	int camId = 0;
	{
		std::unique_ptr<VK4::Vk_Device> device = std::make_unique<VK4::Vk_Device>(name);
		std::unique_ptr<VK4::Vk_Surface> surface = std::make_unique<VK4::Vk_Surface>(device->vk_instance(), name, 500, 300, true);
		std::unique_ptr<VK4::Vk_RenderPass_IM> renderpass = std::make_unique<VK4::Vk_RenderPass_IM>(device.get(), surface.get());
		std::unique_ptr<VK4::Vk_Swapchain_IM> swapchain = std::make_unique<VK4::Vk_Swapchain_IM>(device.get(), surface.get());
		std::unique_ptr<VK4::Vk_Framebuffer_IM> framebuffer = std::make_unique<VK4::Vk_Framebuffer_IM>(device.get(), surface.get(), swapchain.get(), renderpass.get());

		std::string name = "s_dot_p_c";
		std::string fragShaderName = name + ".frag.spv";
		std::string vertShaderName = name + ".vert.spv";
		VK4::Vk_Shader vertShader(device.get(), vertShaderName);
		VK4::Vk_Shader fragShader(device.get(), fragShaderName);

		std::unique_ptr<VK4::Vk_Rasterizer_IM> rasterizer = 
			std::make_unique<VK4::Vk_Rasterizer_IM>(
				camId,
				device.get(),
				VK4::I_Renderer::Vk_PipelineAuxilliaries{
					.surface = surface.get(),
					.renderpass = renderpass.get(),
					.swapchain = swapchain.get(),
					.framebuffer = framebuffer.get()
				},
				3,
				VK4::UniformBufferType_RendererMat4{ 
					.mat = glm::tmat4x4<VK4::point_type>(1.0f) 
				}
			);
		
		std::vector<VkVertexInputBindingDescription> bindingDescription = { VK4::Vk_Vertex_PC::getBindingDescription(0) };
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions = VK4::Vk_Vertex_PC::getAttributeDescriptions(0, 0, 1);

		typedef struct Vk_PushConstants {
			float width;
			float alpha;
			float value3;
		} Vk_PushConstants;

		{
			const VK4::Vk_Config_GraphicsPipeline_IM config {
				.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
				.sizeofPushConstants = sizeof(Vk_PushConstants),
				.cullMode = VK4::CullMode::NoCulling,
				.renderType = VK4::RenderType::Solid,
				.vertexShader = &vertShader,
				.fragmentShader = &fragShader,
				.renderPass = renderpass.get(),
				.descriptorSetLayout = rasterizer->vk_getOrCreate_DescriptorSetLayout(name),
				.bindingDescription = bindingDescription,
				.attributeDescriptions = attributeDescriptions
			};

			std::unique_ptr<VK4::Vk_GraphicsPipeline_IM> pipeline = std::make_unique<VK4::Vk_GraphicsPipeline_IM>(
				camId,
				device.get(), 
				surface.get(),
				config
			);
		}
		{
			const VK4::Vk_Config_GraphicsPipeline_IM config {
				.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
				.sizeofPushConstants = sizeof(Vk_PushConstants),
				.cullMode = VK4::CullMode::Back,
				.renderType = VK4::RenderType::Solid,
				.vertexShader = &vertShader,
				.fragmentShader = &fragShader,
				.renderPass = renderpass.get(),
				.descriptorSetLayout = rasterizer->vk_getOrCreate_DescriptorSetLayout(name),
				.bindingDescription = bindingDescription,
				.attributeDescriptions = attributeDescriptions
			};

			std::unique_ptr<VK4::Vk_GraphicsPipeline_IM> pipeline = std::make_unique<VK4::Vk_GraphicsPipeline_IM>(
				camId,
				device.get(),
				surface.get(),
				config
			);
		}
		{
			const VK4::Vk_Config_GraphicsPipeline_IM config {
				.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
				.sizeofPushConstants = sizeof(Vk_PushConstants),
				.cullMode = VK4::CullMode::Front,
				.renderType = VK4::RenderType::Solid,
				.vertexShader = &vertShader,
				.fragmentShader = &fragShader,
				.renderPass = renderpass.get(),
				.descriptorSetLayout = rasterizer->vk_getOrCreate_DescriptorSetLayout(name),
				.bindingDescription = bindingDescription,
				.attributeDescriptions = attributeDescriptions
			};

			std::unique_ptr<VK4::Vk_GraphicsPipeline_IM> pipeline = std::make_unique<VK4::Vk_GraphicsPipeline_IM>(
				camId,
				device.get(),
				surface.get(),
				config
			);
		}
		{
			const VK4::Vk_Config_GraphicsPipeline_IM config {
				.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
				.sizeofPushConstants = sizeof(Vk_PushConstants),
				.cullMode = VK4::CullMode::NoCulling,
				.renderType = VK4::RenderType::Wireframe,
				.vertexShader = &vertShader,
				.fragmentShader = &fragShader,
				.renderPass = renderpass.get(),
				.descriptorSetLayout = rasterizer->vk_getOrCreate_DescriptorSetLayout(name),
				.bindingDescription = bindingDescription,
				.attributeDescriptions = attributeDescriptions
			};

			std::unique_ptr<VK4::Vk_GraphicsPipeline_IM> pipeline = std::make_unique<VK4::Vk_GraphicsPipeline_IM>(
				camId,
				device.get(),
				surface.get(),
				config
			);
		}
		{
			const VK4::Vk_Config_GraphicsPipeline_IM config {
				.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
				.sizeofPushConstants = sizeof(Vk_PushConstants),
				.cullMode = VK4::CullMode::Back,
				.renderType = VK4::RenderType::Wireframe,
				.vertexShader = &vertShader,
				.fragmentShader = &fragShader,
				.renderPass = renderpass.get(),
				.descriptorSetLayout = rasterizer->vk_getOrCreate_DescriptorSetLayout(name),
				.bindingDescription = bindingDescription,
				.attributeDescriptions = attributeDescriptions
			};

			std::unique_ptr<VK4::Vk_GraphicsPipeline_IM> pipeline = std::make_unique<VK4::Vk_GraphicsPipeline_IM>(
				camId,
				device.get(),
				surface.get(),
				config
			);
		}
		{
			VK4::Vk_Config_GraphicsPipeline_IM config {
				.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
				.sizeofPushConstants = sizeof(Vk_PushConstants),
				.cullMode = VK4::CullMode::Front,
				.renderType = VK4::RenderType::Wireframe,
				.vertexShader = &vertShader,
				.fragmentShader = &fragShader,
				.renderPass = renderpass.get(),
				.descriptorSetLayout = rasterizer->vk_getOrCreate_DescriptorSetLayout(name),
				.bindingDescription = bindingDescription,
				.attributeDescriptions = attributeDescriptions
			};

			std::unique_ptr<VK4::Vk_GraphicsPipeline_IM> pipeline = std::make_unique<VK4::Vk_GraphicsPipeline_IM>(
				camId,
				device.get(),
				surface.get(),
				config
			);
		}
		{
			const VK4::Vk_Config_GraphicsPipeline_IM config {
				.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
				.sizeofPushConstants = sizeof(Vk_PushConstants),
				.cullMode = VK4::CullMode::NoCulling,
				.renderType = VK4::RenderType::Point,
				.vertexShader = &vertShader,
				.fragmentShader = &fragShader,
				.renderPass = renderpass.get(),
				.descriptorSetLayout = rasterizer->vk_getOrCreate_DescriptorSetLayout(name),
				.bindingDescription = bindingDescription,
				.attributeDescriptions = attributeDescriptions
			};

			std::unique_ptr<VK4::Vk_GraphicsPipeline_IM> pipeline = std::make_unique<VK4::Vk_GraphicsPipeline_IM>(
				camId,
				device.get(),
				surface.get(),
				config
			);
		}
		{
			const VK4::Vk_Config_GraphicsPipeline_IM config {
				.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
				.sizeofPushConstants = sizeof(Vk_PushConstants),
				.cullMode = VK4::CullMode::Back,
				.renderType = VK4::RenderType::Point,
				.vertexShader = &vertShader,
				.fragmentShader = &fragShader,
				.renderPass = renderpass.get(),
				.descriptorSetLayout = rasterizer->vk_getOrCreate_DescriptorSetLayout(name),
				.bindingDescription = bindingDescription,
				.attributeDescriptions = attributeDescriptions
			};

			std::unique_ptr<VK4::Vk_GraphicsPipeline_IM> pipeline = std::make_unique<VK4::Vk_GraphicsPipeline_IM>(
				camId,
				device.get(),
				surface.get(),
				config
			);
		}
		{
			const VK4::Vk_Config_GraphicsPipeline_IM config {
				.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
				.sizeofPushConstants = sizeof(Vk_PushConstants),
				.cullMode = VK4::CullMode::Front,
				.renderType = VK4::RenderType::Point,
				.vertexShader = &vertShader,
				.fragmentShader = &fragShader,
				.renderPass = renderpass.get(),
				.descriptorSetLayout = rasterizer->vk_getOrCreate_DescriptorSetLayout(name),
				.bindingDescription = bindingDescription,
				.attributeDescriptions = attributeDescriptions
			};

			std::unique_ptr<VK4::Vk_GraphicsPipeline_IM> pipeline = std::make_unique<VK4::Vk_GraphicsPipeline_IM>(
				camId,
				device.get(),
				surface.get(),
				config
			);
		}
	}
}

BOOST_AUTO_TEST_SUITE_END()