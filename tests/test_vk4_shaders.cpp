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

BOOST_AUTO_TEST_SUITE(RunTestVk4Shaders)

auto new_test = boost::unit_test::enabled();
auto all_tests = boost::unit_test::disabled();

BOOST_AUTO_TEST_CASE(Test_ShaderLoader, *new_test) {
	std::string name = "test_app";

	std::unique_ptr<VK4::Vk_Instance> instance = std::make_unique<VK4::Vk_Instance>(name);
	std::unique_ptr<VK4::Vk_Surface> surface = std::make_unique<VK4::Vk_Surface>(instance.get(), name, 500, 300, true);
	std::unique_ptr<VK4::Vk_Device> device = std::make_unique<VK4::Vk_Device>(name);

	{
		std::string fragShaderName = "s_dot_pc.frag.spv";
		std::string vertShaderName = "s_dot_pc.vert.spv";
		VK4::Vk_Shader vertShader(device.get(), vertShaderName);
		VK4::Vk_Shader fragShader(device.get(), fragShaderName);
	}
}

BOOST_AUTO_TEST_CASE(Test_Shader, *new_test) {
	std::string name = VK4::S_Dot_P_C::Identifier;

	std::unique_ptr<VK4::Vk_Device> device = std::make_unique<VK4::Vk_Device>(name);
	std::unique_ptr<VK4::Vk_Surface> surface = std::make_unique<VK4::Vk_Surface>(device->vk_instance(), name, 500, 300, true, true);

	std::unique_ptr<VK4::Vk_RenderPass_IM> renderpass = std::make_unique<VK4::Vk_RenderPass_IM>(device.get(), surface.get());
	std::unique_ptr<VK4::Vk_Swapchain_IM> swapchain = std::make_unique<VK4::Vk_Swapchain_IM>(device.get(), surface.get());
	std::unique_ptr<VK4::Vk_Framebuffer_IM> framebuffer = std::make_unique<VK4::Vk_Framebuffer_IM>(device.get(), surface.get(), swapchain.get(), renderpass.get());

	LWWS::TViewportId viewportId = 0;
	std::unique_ptr<VK4::Vk_Rasterizer_IM> rasterizer = 
		std::make_unique<VK4::Vk_Rasterizer_IM>(
			viewportId,
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

	{
		auto sets1 = rasterizer->vk_getOrCreate_DescriptorSets(name, 1);
		uint64_t addr1 = reinterpret_cast<uint64_t>(sets1[0]);
		assert(rasterizer->sizeofDescriptorPoolBuffer(name) == 1);
		assert(rasterizer->sizeofDescriptorSetBuffer(name) == 0);
		assert(rasterizer->sizeofLastDescriptorPool(name) == 2);
		assert(sets1.size() == 1);
		assert(rasterizer->sizeofAllDescriptorPools(name) == std::vector<int>({2}));

		rasterizer->vk_reclaim_DescriptorSets(name, sets1);
		assert(rasterizer->sizeofDescriptorPoolBuffer(name) == 1);
		assert(rasterizer->sizeofDescriptorSetBuffer(name) == 1);
		assert(rasterizer->sizeofLastDescriptorPool(name) == 2);
		assert(sets1.size() == 0);
		assert(rasterizer->sizeofAllDescriptorPools(name) == std::vector<int>({2}));

		auto sets2 = rasterizer->vk_getOrCreate_DescriptorSets(name, 2);
		uint64_t addr2 = reinterpret_cast<uint64_t>(sets2[0]);
		assert(addr1 == addr2);
		assert(rasterizer->sizeofDescriptorPoolBuffer(name) == 1);
		assert(rasterizer->sizeofDescriptorSetBuffer(name) == 0);
		assert(rasterizer->sizeofLastDescriptorPool(name) == 1);
		assert(sets2.size() == 2);
		assert(rasterizer->sizeofAllDescriptorPools(name) == std::vector<int>({1}));

		auto sets3 = rasterizer->vk_getOrCreate_DescriptorSets(name, 1);
		assert(rasterizer->sizeofDescriptorPoolBuffer(name) == 1);
		assert(rasterizer->sizeofDescriptorSetBuffer(name) == 0);
		assert(rasterizer->sizeofLastDescriptorPool(name) == 0);
		assert(sets3.size() == 1);
		assert(rasterizer->sizeofAllDescriptorPools(name) == std::vector<int>({0}));

		auto sets4 = rasterizer->vk_getOrCreate_DescriptorSets(name, 1);
		assert(rasterizer->sizeofDescriptorPoolBuffer(name) == 2);
		assert(rasterizer->sizeofDescriptorSetBuffer(name) == 0);
		assert(rasterizer->sizeofLastDescriptorPool(name) == 2);
		assert(sets4.size() == 1);
		assert(rasterizer->sizeofAllDescriptorPools(name) == std::vector<int>({0, 2}));

		auto sets5 = rasterizer->vk_getOrCreate_DescriptorSets(name, 3);
		assert(rasterizer->sizeofDescriptorPoolBuffer(name) == 3);
		assert(rasterizer->sizeofDescriptorSetBuffer(name) == 0);
		assert(rasterizer->sizeofLastDescriptorPool(name) == 2);
		assert(sets5.size() == 3);
		assert(rasterizer->sizeofAllDescriptorPools(name) == std::vector<int>({ 0, 0, 2 }));

		std::vector<uint64_t> b;
		for (auto i : sets2) b.push_back(reinterpret_cast<uint64_t>(i));
		rasterizer->vk_reclaim_DescriptorSets(name, sets2);
		assert(rasterizer->sizeofDescriptorPoolBuffer(name) == 3);
		assert(rasterizer->sizeofDescriptorSetBuffer(name) == 2);
		assert(rasterizer->sizeofLastDescriptorPool(name) == 2);
		assert(sets2.size() == 0);
		assert(rasterizer->sizeofAllDescriptorPools(name) == std::vector<int>({ 0, 0, 2 }));

		for (auto i : sets3) b.push_back(reinterpret_cast<uint64_t>(i));
		rasterizer->vk_reclaim_DescriptorSets(name, sets3);
		assert(rasterizer->sizeofDescriptorPoolBuffer(name) == 3);
		assert(rasterizer->sizeofDescriptorSetBuffer(name) == 3);
		assert(rasterizer->sizeofLastDescriptorPool(name) == 2);
		assert(sets3.size() == 0);
		assert(rasterizer->sizeofAllDescriptorPools(name) == std::vector<int>({ 0, 0, 2 }));

		for (auto i : sets4) b.push_back(reinterpret_cast<uint64_t>(i));
		rasterizer->vk_reclaim_DescriptorSets(name, sets4);
		assert(rasterizer->sizeofDescriptorPoolBuffer(name) == 3);
		assert(rasterizer->sizeofDescriptorSetBuffer(name) == 4);
		assert(rasterizer->sizeofLastDescriptorPool(name) == 2);
		assert(sets4.size() == 0);
		assert(rasterizer->sizeofAllDescriptorPools(name) == std::vector<int>({ 0, 0, 2 }));

		for (auto i : sets5) b.push_back(reinterpret_cast<uint64_t>(i));
		rasterizer->vk_reclaim_DescriptorSets(name, sets5);
		assert(rasterizer->sizeofDescriptorPoolBuffer(name) == 3);
		assert(rasterizer->sizeofDescriptorSetBuffer(name) == 7);
		assert(rasterizer->sizeofLastDescriptorPool(name) == 2);
		assert(sets5.size() == 0);
		assert(rasterizer->sizeofAllDescriptorPools(name) == std::vector<int>({ 0, 0, 2 }));

		auto sets6 = rasterizer->vk_getOrCreate_DescriptorSets(name, 8);
		assert(rasterizer->sizeofDescriptorPoolBuffer(name) == 3);
		assert(rasterizer->sizeofDescriptorSetBuffer(name) == 0);
		assert(rasterizer->sizeofLastDescriptorPool(name) == 1);
		assert(sets6.size() == 8);
		assert(rasterizer->sizeofAllDescriptorPools(name) == std::vector<int>({ 0, 0, 1 }));
		for (int i = 0; i < 7; ++i) {
			assert(b.at(i) == reinterpret_cast<uint64_t>(sets6.at(i)));
		}

		auto sets7 = rasterizer->vk_getOrCreate_DescriptorSets(name, 4);
		assert(rasterizer->sizeofDescriptorPoolBuffer(name) == 4);
		assert(rasterizer->sizeofDescriptorSetBuffer(name) == 0);
		assert(rasterizer->sizeofLastDescriptorPool(name) == 0);
		assert(sets7.size() == 4);
		assert(rasterizer->sizeofAllDescriptorPools(name) == std::vector<int>({ 0, 0, 0, 0 }));

		auto sets9 = rasterizer->vk_getOrCreate_DescriptorSets(name, 4);
		assert(rasterizer->sizeofDescriptorPoolBuffer(name) == 6);
		assert(rasterizer->sizeofDescriptorSetBuffer(name) == 0);
		assert(rasterizer->sizeofLastDescriptorPool(name) == 2);
		assert(sets9.size() == 4);
		assert(rasterizer->sizeofAllDescriptorPools(name) == std::vector<int>({ 0, 0, 0, 0, 0, 2 }));

		auto sets10 = rasterizer->vk_getOrCreate_DescriptorSets(name, 2);
		assert(rasterizer->sizeofDescriptorPoolBuffer(name) == 6);
		assert(rasterizer->sizeofDescriptorSetBuffer(name) == 0);
		assert(rasterizer->sizeofLastDescriptorPool(name) == 0);
		assert(sets10.size() == 2);
		assert(rasterizer->sizeofAllDescriptorPools(name) == std::vector<int>({ 0, 0, 0, 0, 0, 0 }));

		auto sets11 = rasterizer->vk_getOrCreate_DescriptorSets(name, 7);
		assert(rasterizer->sizeofDescriptorPoolBuffer(name) == 9);
		assert(rasterizer->sizeofDescriptorSetBuffer(name) == 0);
		assert(rasterizer->sizeofLastDescriptorPool(name) == 2);
		assert(sets11.size() == 7);
		assert(rasterizer->sizeofAllDescriptorPools(name) == std::vector<int>({ 0, 0, 0, 0, 0, 0, 0, 0, 2 }));
	}
}

BOOST_AUTO_TEST_SUITE_END()