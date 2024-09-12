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
#include "../src/application/Vk_DeviceQueue.hpp"

// #include "../src/vk_renderer4/renderer/rasterizer/immediate_mode/Vk_Graphicspipeline_IM.hpp"

#include "../src/Vk_Viewer.hpp"

#define TEST_SCENARIO_1
#include "test_data.hpp"
#undef TEST_SCENARIO_1
#include "test_utilities.hpp"

BOOST_AUTO_TEST_SUITE(RunTestVk4DeviceQueue)

auto new_test = boost::unit_test::enabled();
auto all_tests = boost::unit_test::disabled();

BOOST_AUTO_TEST_CASE(Test_DeviceQueue1, *all_tests) {
	std::string name = "device_queue_test";
	VK4::Vk_Device device(name);
	VK4::Vk_DeviceQueue queue(&device);

	queue.vk_printAvailableQueueTypes();
}

BOOST_AUTO_TEST_CASE(Test_DeviceQueue2, *new_test) {
	std::string name = "device_queue_test";
	VK4::Vk_Device device(name);
	VK4::Vk_DeviceQueue queue(&device);

	std::unordered_map<LWWS::TViewportId, LWWS::LWWS_Viewport> viewports;
	viewports.insert({0, LWWS::LWWS_Viewport(0, 0,0, 1,1, 0,"#000000", "#FFFFFF")});
	VK4::Vk_Surface surface(device.vk_instance(), name, 1, 1, "#999999", viewports, false);

	queue.vk_determineQueueFamiliesPresentCapability({ .surface=&surface, .viewportId=0 });
	queue.vk_printAvailableQueueTypes();
}

BOOST_AUTO_TEST_SUITE_END()