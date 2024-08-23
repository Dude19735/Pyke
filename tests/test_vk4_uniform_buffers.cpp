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

BOOST_AUTO_TEST_SUITE(RunTestVk4UniformBuffers)

auto new_test = boost::unit_test::enabled();
auto all_tests = boost::unit_test::disabled();

BOOST_AUTO_TEST_CASE(Test_UniformBuffer, *new_test) {
	std::string name = "test_app";

	std::unique_ptr<VK4::Vk_Instance> instance = std::make_unique<VK4::Vk_Instance>(name);
	std::unique_ptr<VK4::Vk_Surface> surface = std::make_unique<VK4::Vk_Surface>(instance.get(), name, 500, 300, true);
	std::unique_ptr<VK4::Vk_Device> device = std::make_unique<VK4::Vk_Device>(name);

	size_t testCount = static_cast<size_t>(std::ceil(std::pow(2, 10)));
	{
		// test matrix
		VK4::point_type proj1[16] = {
			1.0, 0, 0, 0,
			0, 2.0, 0, 0,
			0, 0, 3.0, 4.0,
			0, 0, 5.0, 0
		};
		glm::tmat4x4<VK4::point_type> mat1 = glm::transpose(glm::make_mat4(proj1));
		VK4::UniformBufferType_RendererMat4 ubf1{ mat1 };
		
		VK4::point_type proj2[16] = {
			1.5f, 0, 0, 0,
			0, 2.5f, 0, 0,
			0, 0, 3.5f, 4.5f,
			0, 0, 5.5f, 9.2f
		};
		glm::tmat4x4<VK4::point_type> mat2 = glm::transpose(glm::make_mat4(proj2));
		VK4::UniformBufferType_RendererMat4 ubf2{ mat2 };

		std::uint32_t frameCount = 5;

		std::unique_ptr<VK4::Vk_UniformBuffer<VK4::UniformBufferType_RendererMat4>> buffer = std::make_unique<VK4::Vk_UniformBuffer<VK4::UniformBufferType_RendererMat4>>(device.get(), "TestObj", frameCount, ubf1);
		for (uint32_t i = 0; i < frameCount; ++i) {
			auto back = buffer->vk_getData(i);
			bool same = VK4::UniformBufferType_RendererMat4::compare(ubf1, back);
			assert(same == true);
		}
		for (uint32_t i = 0; i < frameCount; ++i) {
			buffer->vk_update(i, &ubf2);
		}
		for (uint32_t i = 0; i < frameCount; ++i) {
			VK4::UniformBufferType_RendererMat4 back = buffer->vk_getData(i);
			bool same = VK4::UniformBufferType_RendererMat4::compare(ubf2, back);
			assert(same == true);
		}
	}
}

BOOST_AUTO_TEST_SUITE_END()