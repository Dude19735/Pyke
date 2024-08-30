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

BOOST_AUTO_TEST_SUITE(RunTestVk4Camera)

auto new_test = boost::unit_test::disabled();
auto all_tests = boost::unit_test::enabled();

BOOST_AUTO_TEST_CASE(Test_Camera_Init, *all_tests) {
	std::string name = "lolol";
	{
		uint32_t width = 1200;
		uint32_t height = 1024;
		VK4::Vk_Device device(name);
		VK4::Vk_Viewer cam(&device, VK4::Vk_ViewerParams("name", 1200, 1024));

		cam.vk_addCamera(
			std::vector<VK4::Vk_CameraInit> {
			VK4::Vk_CameraInit{
				.camId = 0,
				.viewport = VK4::Vk_Viewport{
					.x = 0, 
					.y = 0, 
					.width = 1200, 
					.height = 1024,
					.clearColor = VK4::Vk_RGBColor {
						.r=0.5, .g=0.25, .b=0.25
					}
				},
				.specs = VK4::Vk_CameraSpecs {
					.type = VK4::Vk_CameraType::Rasterizer_IM,
					.wPos = glm::tvec3<VK4::point_type> {5.0f, 5.0f, 5.0f },
					.wLook = glm::tvec3<VK4::point_type> {0.0f, 0.0f, 0.0f },
					.wUp = glm::tvec3<VK4::point_type> {0.0f, 0.0f, 1.0f },
					.fow = M_PI_2,
					.wNear = 0.1f,
					.wFar = 100.0f
				}
			}
		});

		auto cp = UT::Vk4TestData::Point_P();
		auto cc = UT::Vk4TestData::Point_C();
		auto ci = UT::Vk4TestData::Point_P_C_Indices();
		auto dot = VK4::S_Dot_P_C::create(
			&device,
			"test_object",
			glm::tmat4x4<VK4::point_type> {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1},
			cp, cc, ci, 2.0f, 1.0f
		);
		auto dot2 = VK4::S_Dot_P_C::create(
			&device,
			"test_object_2",
			glm::tmat4x4<VK4::point_type> {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1},
			cp, cc, ci, 2.0f, 1.0f
		);

        cam.vk_runThread();
		cam.vk_attachToAll(dot);
		cam.vk_attachTo(0, dot2);
        cam.vk_rebuildAndRedraw();
        cam.vk_stopThread();
		cam.vk_detachFrom(0, dot2);
		cam.vk_detachFromAll(dot);
	}
}

BOOST_AUTO_TEST_CASE(Test_Camera_Run, *all_tests) {
	std::string name = "lolol";
	{
		uint32_t width = 1800;
		uint32_t height = 900;
		VK4::Vk_Device device(name);
		VK4::Vk_Viewer cam(&device, VK4::Vk_ViewerParams("name", width, height));

		cam.vk_addCamera(
			std::vector<VK4::Vk_CameraInit> {
			VK4::Vk_CameraInit{
				.camId = 0,
				.viewport = VK4::Vk_Viewport{
					.x = 0,
					.y = 0,
					.width = 900,
					.height = 900,
					.clearColor = VK4::Vk_RGBColor {
						.r=0.5, .g=0.0, .b=0.0
					}
				},
				.specs = VK4::Vk_CameraSpecs {
					.type = VK4::Vk_CameraType::Rasterizer_IM,
					.wPos = glm::tvec3<VK4::point_type> {5.0f, 5.0f, 5.0f },
					.wLook = glm::tvec3<VK4::point_type> {0.0f, 0.0f, 0.0f },
					.wUp = glm::tvec3<VK4::point_type> {0.0f, 0.0f, 1.0f },
					.fow = 80.0f / 180.0f * glm::pi<float>(),
					.wNear = 1.0f,
					.wFar = 100.0f
				}
			},
			VK4::Vk_CameraInit{
				.camId = 1,
				.viewport = VK4::Vk_Viewport{
					.x = 900, 
					.y = 0, 
					.width = 900, 
					.height = 900,
					.clearColor = VK4::Vk_RGBColor {
						.r=0.0, .g=0.0, .b=1.0
					}
				},
				.specs = VK4::Vk_CameraSpecs {
					.type = VK4::Vk_CameraType::Rasterizer_IM,
					.wPos = glm::tvec3<VK4::point_type> {5.0f, 0.0f, 5.0f },
					.wLook = glm::tvec3<VK4::point_type> {0.0f, 0.0f, 0.0f },
					.wUp = glm::tvec3<VK4::point_type> {0.0f, 0.0f, 1.0f },
					.fow = 80.0f / 180.0f * glm::pi<float>(),
					.wNear = 1.0f,
					.wFar = 100.0f
				}
			}
		});

		auto cp = UT::Vk4TestData::Point_P();
		auto cc = UT::Vk4TestData::Point_C();
		auto ci = UT::Vk4TestData::Point_P_C_Indices();
		auto dot = VK4::S_Dot_P_C::create(
			&device,
			"test_object",
			glm::tmat4x4<VK4::point_type> {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1},
			cp, cc, ci, 2.0f, 1.0f
		);

        cam.vk_runThread();
		cam.vk_attachToAll(dot);
		cam.vk_rebuildAndRedraw();
        cam.vk_stopThread();
        // while(cam.vk_running()) std::this_thread::sleep_for(std::chrono::milliseconds(500));
		cam.vk_detachFromAll(dot);
        
	}
}

BOOST_AUTO_TEST_CASE(Test_Camera_Scale, *new_test) {
	std::string name = "lolol";
	{
		uint32_t width = 1800;
		uint32_t height = 1000;
		VK4::Vk_Device device(name);
		VK4::Vk_Viewer cam(&device, VK4::Vk_ViewerParams("name", width, height));

		cam.vk_addCamera(
			std::vector<VK4::Vk_CameraInit> {
			VK4::Vk_CameraInit{
				.camId = 0,
				.viewport = VK4::Vk_Viewport{
					.x = 100,
					.y = 25,
					.width = 500,
					.height = 400,
					.clearColor = VK4::Vk_RGBColor {
						.r=0.5, .g=0.25, .b=0.25
					}
				},
				.specs = VK4::Vk_CameraSpecs {
					.type = VK4::Vk_CameraType::Rasterizer_IM,
					.wPos = glm::tvec3<VK4::point_type> {5.0f, 5.0f, 5.0f },
					.wLook = glm::tvec3<VK4::point_type> {0.0f, 0.0f, 0.0f },
					.wUp = glm::tvec3<VK4::point_type> {0.0f, 0.0f, 1.0f },
					.fow = 80.0f / 180.0f * glm::pi<float>(),
					.wNear = 1.0f,
					.wFar = 100.0f
				}
			},
			VK4::Vk_CameraInit{
				.camId = 1,
				.viewport = VK4::Vk_Viewport{
					.x = 650,
					.y = 50,
					.width = 450,
					.height = 450,
					.clearColor = VK4::Vk_RGBColor {
						.r=0.75, .g=0.75, .b=0.75
					}
				},
				.specs = VK4::Vk_CameraSpecs {
					.type = VK4::Vk_CameraType::Rasterizer_IM,
					.wPos = glm::tvec3<VK4::point_type> {5.0f, 0.0f, 5.0f },
					.wLook = glm::tvec3<VK4::point_type> {0.0f, 0.0f, 0.0f },
					.wUp = glm::tvec3<VK4::point_type> {0.0f, 0.0f, 1.0f },
					.fow = 80.0f / 180.0f * glm::pi<float>(),
					.wNear = 1.0f,
					.wFar = 100.0f
				}
			}
		});

		auto cp = UT::Vk4TestData::Point_P();
		auto cc = UT::Vk4TestData::Point_C();
		auto ci = UT::Vk4TestData::Point_P_C_Indices();
		auto dot = VK4::S_Dot_P_C::create(
			&device,
			"test_object",
			glm::tmat4x4<VK4::point_type> {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1},
			cp, cc, ci, 2.0f, 1.0f
		);

        cam.vk_runThread();
		cam.vk_attachToAll(dot);
		cam.vk_rebuildAndRedraw();
        
		cam.vk_detachFromAll(dot);
	}
}

BOOST_AUTO_TEST_CASE(Test_GridLayout, *all_tests) {
	VK4::I_LayoutPack pack {
		.specs=VK4::Vk_CameraSpecs {
			.type = VK4::Vk_CameraType::Rasterizer_IM,
			.wPos = glm::tvec3<VK4::point_type> {5.0f, 5.0f, 5.0f },
			.wLook = glm::tvec3<VK4::point_type> {0.0f, 0.0f, 0.0f },
			.wUp = glm::tvec3<VK4::point_type> {0.0f, 0.0f, 1.0f },
			.fow = 80.0f / 180.0f * glm::pi<float>(),
			.wNear = 1.0f,
			.wFar = 100.0f
		},
		.clearColor=VK4::Vk_RGBColor {
			.r=0.75, .g=0.75, .b=0.75
		}
	};

	{
		int xCount = 3; // this is width-direction
		int yCount = 4; // this is height-direction
		int res;
		VK4::Vk_GridLayout layout(xCount, yCount, 0, 0);
		BOOST_ASSERT(layout.vk_count() == 0);
		res = layout.vk_addCamera(0, 0, pack, false);
		BOOST_ASSERT(res == 1);
		BOOST_ASSERT(layout.vk_count() == 1);
		res = layout.vk_addCamera(0, 0, pack, false);
		BOOST_ASSERT(res == 0);
		BOOST_ASSERT(layout.vk_count() == 1);
		
		res = layout.vk_addCamera(0, 4, pack, false);
		BOOST_ASSERT(res == 0);
		BOOST_ASSERT(layout.vk_count() == 1);
		res = layout.vk_addCamera(3, 0, pack, false);
		BOOST_ASSERT(res == 0);
		BOOST_ASSERT(layout.vk_count() == 1);
		
		res = layout.vk_addCamera(0, 0, pack, true);
		BOOST_ASSERT(res == 1);
		BOOST_ASSERT(layout.vk_count() == 1);
		res = layout.vk_addCamera(0, 1, pack, false);
		BOOST_ASSERT(res == 1);
		BOOST_ASSERT(layout.vk_count() == 2);
		res = layout.vk_addCamera(0, 2, pack, false);
		BOOST_ASSERT(res == 1);
		BOOST_ASSERT(layout.vk_count() == 3);
		res = layout.vk_addCamera(0, 3, pack, false);
		BOOST_ASSERT(res == 1);
		BOOST_ASSERT(layout.vk_count() == 4);

		res = layout.vk_addCamera(1, 0, pack, true);
		BOOST_ASSERT(res == 1);
		BOOST_ASSERT(layout.vk_count() == 5);
		res = layout.vk_addCamera(1, 1, pack, false);
		BOOST_ASSERT(res == 1);
		BOOST_ASSERT(layout.vk_count() == 6);
		res = layout.vk_addCamera(1, 2, pack, false);
		BOOST_ASSERT(res == 1);
		BOOST_ASSERT(layout.vk_count() == 7);
		res = layout.vk_addCamera(1, 3, pack, false);
		BOOST_ASSERT(res == 1);
		BOOST_ASSERT(layout.vk_count() == 8);

		res = layout.vk_addCamera(2, 0, pack, true);
		BOOST_ASSERT(res == 1);
		BOOST_ASSERT(layout.vk_count() == 9);
		res = layout.vk_addCamera(2, 1, pack, false);
		BOOST_ASSERT(res == 1);
		BOOST_ASSERT(layout.vk_count() == 10);
		res = layout.vk_addCamera(2, 2, pack, false);
		BOOST_ASSERT(res == 1);
		BOOST_ASSERT(layout.vk_count() == 11);
		res = layout.vk_addCamera(2, 3, pack, false);
		BOOST_ASSERT(res == 1);
		BOOST_ASSERT(layout.vk_count() == 12);

		res = layout.vk_addCamera(1, 0, pack, true);
		BOOST_ASSERT(res == 1);
		BOOST_ASSERT(layout.vk_count() == 12);
	}
	{
		int yCount = 4;
		int xCount = 3;
		int res;
		VK4::Vk_GridLayout layout(xCount, yCount, 0, 0);
		res = layout.vk_addCamera(0, 0, {
			pack, pack, pack, pack, pack, pack, pack, pack, pack, pack, pack, pack
		}, false);
		BOOST_ASSERT(res == 12);
		BOOST_ASSERT(layout.vk_count() == 12);
	}
	{
		int yCount = 4;
		int xCount = 3;
		int res;
		VK4::Vk_GridLayout layout(xCount, yCount, 0, 0);
		res = layout.vk_addCamera(0, 0, {
			pack, pack, pack, pack, pack, pack, pack, pack, pack, pack, pack, pack, pack
		}, false);
		BOOST_ASSERT(res == 12);
		BOOST_ASSERT(layout.vk_count() == 12);
	}
	{
		int yCount = 4;
		int xCount = 3;
		int res;
		VK4::Vk_GridLayout layout(xCount, yCount, 0, 0);
		
		res = layout.vk_addCamera(2, 3, pack, false);
		BOOST_ASSERT(res == 1);
		BOOST_ASSERT(layout.vk_count() == 1);
		
		res = layout.vk_addCamera(0, 0, {
			pack, pack, pack, pack, pack, pack, pack, pack, pack, pack, pack
		}, false);
		BOOST_ASSERT(res == 11);
		BOOST_ASSERT(layout.vk_count() == 12);
	}
	{
		int yCount = 4;
		int xCount = 3;
		int res;
		VK4::Vk_GridLayout layout(xCount, yCount, 0, 0);
		
		res = layout.vk_addCamera(2, 3, pack, false);
		BOOST_ASSERT(res == 1);
		BOOST_ASSERT(layout.vk_count() == 1);
		
		res = layout.vk_addCamera(0, 0, {
			pack, pack, pack, pack, pack, pack, pack, pack, pack, pack, pack, pack
		}, false);
		BOOST_ASSERT(res == 11);
		BOOST_ASSERT(layout.vk_count() == 12);
	}
	{
		int yCount = 4;
		int xCount = 3;
		int res;
		VK4::Vk_GridLayout layout(xCount, yCount, 0, 0);
		
		res = layout.vk_addCamera(2, 3, pack, false);
		BOOST_ASSERT(res == 1);
		BOOST_ASSERT(layout.vk_count() == 1);
		
		res = layout.vk_addCamera(0, 0, {
			pack, pack, pack, pack, pack, pack, pack, pack, pack, pack, pack, pack
		}, true);
		BOOST_ASSERT(res == 12);
		BOOST_ASSERT(layout.vk_count() == 12);
	}
	{
		int yCount = 4;
		int xCount = 3;
		int res;
		VK4::Vk_GridLayout layout(xCount, yCount, 0, 0);
		
		res = layout.vk_addCamera(2, 3, pack, false);
		BOOST_ASSERT(res == 1);
		BOOST_ASSERT(layout.vk_count() == 1);
		
		res = layout.vk_addCamera(0, 0, {
			pack, pack, pack, pack, pack, pack, pack, pack
		}, true);
		BOOST_ASSERT(res == 8);
		BOOST_ASSERT(layout.vk_count() == 9);
	}
	{
		int yCount = 4;
		int xCount = 3;
		int res;
		VK4::Vk_GridLayout layout(xCount, yCount, 0, 0);
		
		res = layout.vk_addCamera(2, 3, pack, false);
		BOOST_ASSERT(res == 1);
		BOOST_ASSERT(layout.vk_count() == 1);
		
		res = layout.vk_addCamera(2, 3, {
			pack, pack
		}, false);
		BOOST_ASSERT(res == 0);
		BOOST_ASSERT(layout.vk_count() == 1);
	}
	{
		int yCount = 4;
		int xCount = 3;
		int res;
		VK4::Vk_GridLayout layout(xCount, yCount, 0, 0);
		
		res = layout.vk_addCamera(2, 3, pack, false);
		BOOST_ASSERT(res == 1);
		BOOST_ASSERT(layout.vk_count() == 1);
		
		res = layout.vk_addCamera(2, 3, {
			pack, pack
		}, true);
		BOOST_ASSERT(res == 1);
		BOOST_ASSERT(layout.vk_count() == 1);
	}
	{
		int yCount = 4;
		int xCount = 3;
		int res;
		VK4::Vk_GridLayout layout(xCount, yCount, 0, 0);
		
		res = layout.vk_addCamera(2, 3, pack, false);
		BOOST_ASSERT(res == 1);
		BOOST_ASSERT(layout.vk_count() == 1);
		
		res = layout.vk_addCamera(2, 2, {
			pack, pack
		}, false);
		BOOST_ASSERT(res == 2);
		BOOST_ASSERT(layout.vk_count() == 3);
	}
	{
		int yCount = 4;
		int xCount = 3;
		int res;
		VK4::Vk_GridLayout layout(xCount, yCount, 0, 0);
		
		res = layout.vk_addCamera(2, 3, pack, false);
		BOOST_ASSERT(res == 1);
		BOOST_ASSERT(layout.vk_count() == 1);
		
		res = layout.vk_addCamera(2, 2, {
			pack, pack
		}, true);
		BOOST_ASSERT(res == 2);
		BOOST_ASSERT(layout.vk_count() == 3);
	}
	{
		int yCount = 4;
		int xCount = 3;
		int res;
		VK4::Vk_GridLayout layout(xCount, yCount, 0, 0);
		
		res = layout.vk_addCamera(1, 1, pack, false);
		BOOST_ASSERT(res == 1);
		BOOST_ASSERT(layout.vk_count() == 1);
		
		res = layout.vk_addCamera(1, 2, {
			pack, pack, pack, pack, pack, pack, pack
		}, false);
		BOOST_ASSERT(res == 5);
		BOOST_ASSERT(layout.vk_count() == 6);
	}
	{
		int yCount = 4;
		int xCount = 3;
		int res;
		VK4::Vk_GridLayout layout(xCount, yCount, 0, 0);
		
		res = layout.vk_addCamera(1, 1, pack, false);
		BOOST_ASSERT(res == 1);
		BOOST_ASSERT(layout.vk_count() == 1);
		
		res = layout.vk_addCamera(1, 2, {
			pack, pack, pack, pack, pack, pack, pack
		}, true);
		BOOST_ASSERT(res == 5);
		BOOST_ASSERT(layout.vk_count() == 6);
	}
	{
		int yCount = 4;
		int xCount = 3;
		int res;
		VK4::Vk_GridLayout layout(xCount, yCount, 0, 0);
		
		res = layout.vk_addCamera(1, 1, pack, false);
		BOOST_ASSERT(res == 1);
		BOOST_ASSERT(layout.vk_count() == 1);
		
		res = layout.vk_addCamera(1, 1, {
			pack, pack, pack, pack, pack, pack
		}, false);
		BOOST_ASSERT(res == 6);
		BOOST_ASSERT(layout.vk_count() == 7);
	}
	{
		int yCount = 4;
		int xCount = 3;
		int res;
		VK4::Vk_GridLayout layout(xCount, yCount, 0, 0);
		
		res = layout.vk_addCamera(1, 1, pack, false);
		BOOST_ASSERT(res == 1);
		BOOST_ASSERT(layout.vk_count() == 1);
		
		res = layout.vk_addCamera(1, 1, {
			pack, pack, pack, pack, pack, pack
		}, true);
		BOOST_ASSERT(res == 6);
		BOOST_ASSERT(layout.vk_count() == 6);
	}
}

BOOST_AUTO_TEST_CASE(Test_GridLayout_2, *all_tests) {
	VK4::I_LayoutPack pack {
		.specs=VK4::Vk_CameraSpecs {
			.type = VK4::Vk_CameraType::Rasterizer_IM,
			.wPos = glm::tvec3<VK4::point_type> {5.0f, 5.0f, 5.0f },
			.wLook = glm::tvec3<VK4::point_type> {0.0f, 0.0f, 0.0f },
			.wUp = glm::tvec3<VK4::point_type> {0.0f, 0.0f, 1.0f },
			.fow = 80.0f / 180.0f * glm::pi<float>(),
			.wNear = 1.0f,
			.wFar = 100.0f
		},
		.clearColor=VK4::Vk_RGBColor {
			.r=0.75, .g=0.75, .b=0.75
		}
	};

	int xCount = 4; // this is width-direction
	int yCount = 3; // this is height-direction
	{
		int res;
		VK4::Vk_GridLayout layout(xCount, yCount, 0, 0);
		BOOST_ASSERT(layout.vk_count() == 0);
		res = layout.vk_addCamera(0, 0, pack, false);
		BOOST_ASSERT(res == 1);
		BOOST_ASSERT(layout.vk_count() == 1);
		res = layout.vk_addCamera(0, 0, pack, false);
		BOOST_ASSERT(res == 0);
		BOOST_ASSERT(layout.vk_count() == 1);
		
		res = layout.vk_addCamera(0, 3, pack, false);
		BOOST_ASSERT(res == 0);
		BOOST_ASSERT(layout.vk_count() == 1);
		res = layout.vk_addCamera(4, 0, pack, false);
		BOOST_ASSERT(res == 0);
		BOOST_ASSERT(layout.vk_count() == 1);
		
		res = layout.vk_addCamera(0, 0, pack, true);
		BOOST_ASSERT(res == 1);
		BOOST_ASSERT(layout.vk_count() == 1);
		res = layout.vk_addCamera(0, 1, pack, false);
		BOOST_ASSERT(res == 1);
		BOOST_ASSERT(layout.vk_count() == 2);
		res = layout.vk_addCamera(0, 2, pack, false);
		BOOST_ASSERT(res == 1);
		BOOST_ASSERT(layout.vk_count() == 3);

		res = layout.vk_addCamera(1, 0, pack, true);
		BOOST_ASSERT(res == 1);
		BOOST_ASSERT(layout.vk_count() == 4);
		res = layout.vk_addCamera(1, 1, pack, false);
		BOOST_ASSERT(res == 1);
		BOOST_ASSERT(layout.vk_count() == 5);
		res = layout.vk_addCamera(1, 2, pack, false);
		BOOST_ASSERT(res == 1);
		BOOST_ASSERT(layout.vk_count() == 6);

		res = layout.vk_addCamera(2, 0, pack, true);
		BOOST_ASSERT(res == 1);
		BOOST_ASSERT(layout.vk_count() == 7);
		res = layout.vk_addCamera(2, 1, pack, false);
		BOOST_ASSERT(res == 1);
		BOOST_ASSERT(layout.vk_count() == 8);
		res = layout.vk_addCamera(2, 2, pack, false);
		BOOST_ASSERT(res == 1);
		BOOST_ASSERT(layout.vk_count() == 9);

		res = layout.vk_addCamera(3, 0, pack, true);
		BOOST_ASSERT(res == 1);
		BOOST_ASSERT(layout.vk_count() == 10);
		res = layout.vk_addCamera(3, 1, pack, false);
		BOOST_ASSERT(res == 1);
		BOOST_ASSERT(layout.vk_count() == 11);
		res = layout.vk_addCamera(3, 2, pack, false);
		BOOST_ASSERT(res == 1);
		BOOST_ASSERT(layout.vk_count() == 12);

		res = layout.vk_addCamera(1, 0, pack, true);
		BOOST_ASSERT(res == 1);
		BOOST_ASSERT(layout.vk_count() == 12);
	}
	{
		int res;
		VK4::Vk_GridLayout layout(xCount, yCount, 0, 0);
		res = layout.vk_addCamera(0, 0, {
			pack, pack, pack, pack, pack, pack, pack, pack, pack, pack, pack, pack
		}, false);
		BOOST_ASSERT(res == 12);
		BOOST_ASSERT(layout.vk_count() == 12);
	}
	{
		int res;
		VK4::Vk_GridLayout layout(xCount, yCount, 0, 0);
		res = layout.vk_addCamera(0, 0, {
			pack, pack, pack, pack, pack, pack, pack, pack, pack, pack, pack, pack, pack
		}, false);
		BOOST_ASSERT(res == 12);
		BOOST_ASSERT(layout.vk_count() == 12);
	}
	{
		int res;
		VK4::Vk_GridLayout layout(xCount, yCount, 0, 0);
		
		res = layout.vk_addCamera(3, 2, pack, false);
		BOOST_ASSERT(res == 1);
		BOOST_ASSERT(layout.vk_count() == 1);
		
		res = layout.vk_addCamera(0, 0, {
			pack, pack, pack, pack, pack, pack, pack, pack, pack, pack, pack
		}, false);
		BOOST_ASSERT(res == 11);
		BOOST_ASSERT(layout.vk_count() == 12);
	}
	{
		int res;
		VK4::Vk_GridLayout layout(xCount, yCount, 0, 0);
		
		res = layout.vk_addCamera(3, 2, pack, false);
		BOOST_ASSERT(res == 1);
		BOOST_ASSERT(layout.vk_count() == 1);
		
		res = layout.vk_addCamera(0, 0, {
			pack, pack, pack, pack, pack, pack, pack, pack, pack, pack, pack, pack
		}, false);
		BOOST_ASSERT(res == 11);
		BOOST_ASSERT(layout.vk_count() == 12);
	}
	{
		int res;
		VK4::Vk_GridLayout layout(xCount, yCount, 0, 0);
		
		res = layout.vk_addCamera(3, 2, pack, false);
		BOOST_ASSERT(res == 1);
		BOOST_ASSERT(layout.vk_count() == 1);
		
		res = layout.vk_addCamera(0, 0, {
			pack, pack, pack, pack, pack, pack, pack, pack, pack, pack, pack, pack
		}, true);
		BOOST_ASSERT(res == 12);
		BOOST_ASSERT(layout.vk_count() == 12);
	}
	{
		int res;
		VK4::Vk_GridLayout layout(xCount, yCount, 0, 0);
		
		res = layout.vk_addCamera(3, 2, pack, false);
		BOOST_ASSERT(res == 1);
		BOOST_ASSERT(layout.vk_count() == 1);
		
		res = layout.vk_addCamera(0, 0, {
			pack, pack, pack, pack, pack, pack, pack, pack
		}, true);
		BOOST_ASSERT(res == 8);
		BOOST_ASSERT(layout.vk_count() == 9);
	}
	{
		int res;
		VK4::Vk_GridLayout layout(xCount, yCount, 0, 0);
		
		res = layout.vk_addCamera(3, 2, pack, false);
		BOOST_ASSERT(res == 1);
		BOOST_ASSERT(layout.vk_count() == 1);
		
		res = layout.vk_addCamera(3, 2, {
			pack, pack
		}, false);
		BOOST_ASSERT(res == 0);
		BOOST_ASSERT(layout.vk_count() == 1);
	}
	{
		int res;
		VK4::Vk_GridLayout layout(xCount, yCount, 0, 0);
		
		res = layout.vk_addCamera(3, 2, pack, false);
		BOOST_ASSERT(res == 1);
		BOOST_ASSERT(layout.vk_count() == 1);
		
		res = layout.vk_addCamera(3, 2, {
			pack, pack
		}, true);
		BOOST_ASSERT(res == 1);
		BOOST_ASSERT(layout.vk_count() == 1);
	}
	{
		int res;
		VK4::Vk_GridLayout layout(xCount, yCount, 0, 0);
		
		res = layout.vk_addCamera(3, 2, pack, false);
		BOOST_ASSERT(res == 1);
		BOOST_ASSERT(layout.vk_count() == 1);
		
		res = layout.vk_addCamera(2, 2, {
			pack, pack
		}, false);
		BOOST_ASSERT(res == 1);
		BOOST_ASSERT(layout.vk_count() == 2);
	}
	{
		int res;
		VK4::Vk_GridLayout layout(xCount, yCount, 0, 0);
		
		res = layout.vk_addCamera(3, 2, pack, false);
		BOOST_ASSERT(res == 1);
		BOOST_ASSERT(layout.vk_count() == 1);
		
		res = layout.vk_addCamera(2, 2, {
			pack, pack
		}, true);
		BOOST_ASSERT(res == 2);
		BOOST_ASSERT(layout.vk_count() == 2);
	}
	{
		int res;
		VK4::Vk_GridLayout layout(xCount, yCount, 0, 0);
		
		res = layout.vk_addCamera(1, 1, pack, false);
		BOOST_ASSERT(res == 1);
		BOOST_ASSERT(layout.vk_count() == 1);
		
		res = layout.vk_addCamera(2, 1, {
			pack, pack, pack, pack, pack, pack, pack
		}, false);
		BOOST_ASSERT(res == 6);
		BOOST_ASSERT(layout.vk_count() == 7);
	}
	{
		int res;
		VK4::Vk_GridLayout layout(xCount, yCount, 0, 0);
		
		res = layout.vk_addCamera(1, 1, pack, false);
		BOOST_ASSERT(res == 1);
		BOOST_ASSERT(layout.vk_count() == 1);
		
		res = layout.vk_addCamera(2, 1, {
			pack, pack, pack, pack, pack, pack, pack
		}, true);
		BOOST_ASSERT(res == 6);
		BOOST_ASSERT(layout.vk_count() == 7);
	}
	{
		int res;
		VK4::Vk_GridLayout layout(xCount, yCount, 0, 0);
		
		res = layout.vk_addCamera(1, 1, pack, false);
		BOOST_ASSERT(res == 1);
		BOOST_ASSERT(layout.vk_count() == 1);
		
		res = layout.vk_addCamera(1, 1, {
			pack, pack, pack, pack, pack, pack
		}, false);
		BOOST_ASSERT(res == 6);
		BOOST_ASSERT(layout.vk_count() == 7);
	}
	{
		int res;
		VK4::Vk_GridLayout layout(xCount, yCount, 0, 0);
		
		res = layout.vk_addCamera(1, 1, pack, false);
		BOOST_ASSERT(res == 1);
		BOOST_ASSERT(layout.vk_count() == 1);
		
		res = layout.vk_addCamera(1, 1, {
			pack, pack, pack, pack, pack, pack
		}, true);
		BOOST_ASSERT(res == 6);
		BOOST_ASSERT(layout.vk_count() == 6);
	}
}

BOOST_AUTO_TEST_CASE(Test_GridLayout_Global, *all_tests) {
	std::string name = "Test-GridLayout";
	{
		uint32_t width = 1024;
		uint32_t height = 800;
		VK4::Vk_Device device(name);
		VK4::Vk_Viewer cam(&device, VK4::Vk_ViewerParams("name", width, height, VK4::Vk_ViewingType::GLOBAL));

		VK4::Vk_CameraSpecs specs {
			.type = VK4::Vk_CameraType::Rasterizer_IM,
			.wPos = glm::tvec3<VK4::point_type> {5.0f, 5.0f, 5.0f },
			.wLook = glm::tvec3<VK4::point_type> {0.0f, 0.0f, 0.0f },
			.wUp = glm::tvec3<VK4::point_type> {0.0f, 0.0f, 1.0f },
			.fow = 80.0f / 180.0f * glm::pi<float>(),
			.wNear = 1.0f,
			.wFar = 100.0f
		};

		VK4::Vk_RGBColor from { .r=0.5f, .g=0.0f, .b=0.0f };
		VK4::Vk_RGBColor to { .r=0.0f, .g=0.0f, .b=0.5f };

		VK4::Vk_GridLayout layout(3, 3, 25, 25, {
			{specs, VK4::Vk_ColorOp::rgb_lerp(0.0f, from, to)},
			{specs, VK4::Vk_ColorOp::rgb_lerp(0.125f, from, to)},
			{specs, VK4::Vk_ColorOp::rgb_lerp(0.25f, from, to)},
			{specs, VK4::Vk_ColorOp::rgb_lerp(0.375f, from, to)},
			{specs, VK4::Vk_ColorOp::rgb_lerp(0.5f, from, to)},
			{specs, VK4::Vk_ColorOp::rgb_lerp(0.625f, from, to)},
			{specs, VK4::Vk_ColorOp::rgb_lerp(0.75f, from, to)},
			{specs, VK4::Vk_ColorOp::rgb_lerp(0.875f, from, to)},
			{specs, VK4::Vk_ColorOp::rgb_lerp(1.0f, from, to)}
		});

		cam.vk_addCamera(layout.vk_layoutList(width, height));

		auto cp = UT::Vk4TestData::Point_P();
		auto cc = UT::Vk4TestData::Point_C();
		auto ci = UT::Vk4TestData::Point_P_C_Indices();
		auto dot = VK4::S_Dot_P_C::create(
			&device,
			"test_object",
			glm::tmat4x4<VK4::point_type> {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1},
			cp, cc, ci, 2.0f, 1.0f
		);

        cam.vk_runThread();
		cam.vk_attachToAll(dot);
		cam.vk_rebuildAndRedraw();
		cam.vk_detachFromAll(dot);
        
	}
}

BOOST_AUTO_TEST_CASE(Test_GridLayout_Local, *all_tests) {
	std::string name = "Test-GridLayout";
	{
		uint32_t width = 1400;
		uint32_t height = 1400;
		VK4::Vk_Device device(name);
		VK4::Vk_Viewer cam(&device, VK4::Vk_ViewerParams("name", width, height, VK4::Vk_ViewingType::LOCAL));

		VK4::Vk_CameraSpecs specs {
			.type = VK4::Vk_CameraType::Rasterizer_IM,
			.wPos = glm::tvec3<VK4::point_type> {5.0f, 5.0f, 5.0f },
			.wLook = glm::tvec3<VK4::point_type> {0.0f, 0.0f, 0.0f },
			.wUp = glm::tvec3<VK4::point_type> {0.0f, 0.0f, 1.0f },
			.fow = 80.0f / 180.0f * glm::pi<float>(),
			.wNear = 1.0f,
			.wFar = 100.0f
		};

		VK4::Vk_RGBColor from { .r=0.5f, .g=0.0f, .b=0.0f };
		VK4::Vk_RGBColor to { .r=0.0f, .g=0.0f, .b=0.5f };

		VK4::Vk_GridLayout layout(3, 3, 25, 25, {
			{specs, VK4::Vk_ColorOp::rgb_lerp(0.0f, from, to)},
			{specs, VK4::Vk_ColorOp::rgb_lerp(0.125f, from, to)},
			{specs, VK4::Vk_ColorOp::rgb_lerp(0.25f, from, to)},
			{specs, VK4::Vk_ColorOp::rgb_lerp(0.375f, from, to)},
			{specs, VK4::Vk_ColorOp::rgb_lerp(0.5f, from, to)},
			{specs, VK4::Vk_ColorOp::rgb_lerp(0.625f, from, to)},
			{specs, VK4::Vk_ColorOp::rgb_lerp(0.75f, from, to)},
			{specs, VK4::Vk_ColorOp::rgb_lerp(0.875f, from, to)},
			{specs, VK4::Vk_ColorOp::rgb_lerp(1.0f, from, to)}
		});

		cam.vk_addCamera(layout.vk_layoutList(width, height));

		auto cp = UT::Vk4TestData::Point_P();
		auto cc = UT::Vk4TestData::Point_C();
		auto ci = UT::Vk4TestData::Point_P_C_Indices();
		auto dot = VK4::S_Dot_P_C::create(
			&device,
			"test_object",
			glm::tmat4x4<VK4::point_type> {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1},
			cp, cc, ci, 2.0f, 1.0f
		);

        cam.vk_runThread();
		cam.vk_attachToAll(dot);
		cam.vk_rebuildAndRedraw();
        
		cam.vk_detachFromAll(dot);
	}
}

BOOST_AUTO_TEST_CASE(Test_GridLayout_Local_Irregular, *all_tests) {
	std::string name = "Test-GridLayout";
	{
		uint32_t width = 1024;
		uint32_t height = 800;
		VK4::Vk_Device device(name);
		VK4::Vk_Viewer cam(&device, VK4::Vk_ViewerParams("name", width, height, VK4::Vk_ViewingType::LOCAL));

		VK4::Vk_CameraSpecs specs {
			.type = VK4::Vk_CameraType::Rasterizer_IM,
			.wPos = glm::tvec3<VK4::point_type> {5.0f, 5.0f, 5.0f },
			.wLook = glm::tvec3<VK4::point_type> {0.0f, 0.0f, 0.0f },
			.wUp = glm::tvec3<VK4::point_type> {0.0f, 0.0f, 1.0f },
			.fow = 80.0f / 180.0f * glm::pi<float>(),
			.wNear = 1.0f,
			.wFar = 100.0f
		};

		VK4::Vk_RGBColor from { .r=0.5f, .g=0.0f, .b=0.0f };
		VK4::Vk_RGBColor to { .r=0.0f, .g=0.0f, .b=0.5f };

		VK4::Vk_GridLayout layout(3, 3, 0, 0);
		layout.vk_addCamera(0,0, {specs, VK4::Vk_ColorOp::rgb_lerp(0.0f, from, to)});
		layout.vk_addCamera(1,1, {specs, VK4::Vk_ColorOp::rgb_lerp(0.5f, from, to)});
		layout.vk_addCamera(2,2, {specs, VK4::Vk_ColorOp::rgb_lerp(1.0f, from, to)});
		layout.vk_addCamera(2,0, {specs, VK4::Vk_ColorOp::rgb_lerp(0.375f, from, to)});
		layout.vk_addCamera(0,2, {specs, VK4::Vk_ColorOp::rgb_lerp(0.750f, from, to)});

		cam.vk_addCamera(layout.vk_layoutList(width, height));

		auto cp = UT::Vk4TestData::Point_P();
		auto cc = UT::Vk4TestData::Point_C();
		auto ci = UT::Vk4TestData::Point_P_C_Indices();
		auto dot = VK4::S_Dot_P_C::create(
			&device,
			"test_object",
			glm::tmat4x4<VK4::point_type> {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1},
			cp, cc, ci, 2.0f, 1.0f
		);

        cam.vk_runThread();
		cam.vk_attachToAll(dot);
		cam.vk_rebuildAndRedraw();
		cam.vk_detachFromAll(dot);
        
	}
}

BOOST_AUTO_TEST_CASE(Test_GridLayout_Local_ObjectCentric, *all_tests) {
	std::string name = "Test-GridLayout-ObjectCentric";
	{
		uint32_t width = 1024;
		uint32_t height = 800;
		VK4::Vk_Device device(name);
		VK4::Vk_Viewer cam(&device, VK4::Vk_ViewerParams("name", width, height, VK4::Vk_ViewingType::LOCAL));

		VK4::Vk_CameraSpecs specs_ObjectCentric {
			.type = VK4::Vk_CameraType::Rasterizer_IM,
			.wPos = glm::tvec3<VK4::point_type> {5.0f, 5.0f, 5.0f },
			.wLook = glm::tvec3<VK4::point_type> {0.0f, 0.0f, 0.0f },
			.wUp = glm::tvec3<VK4::point_type> {0.0f, 0.0f, 1.0f },
			.fow = 80.0f / 180.0f * glm::pi<float>(),
			.wNear = 1.0f,
			.wFar = 100.0f,
			.steeringType = VK4::Vk_SteeringType::OBJECT_CENTRIC
		};

		VK4::Vk_CameraSpecs specs_CameraCentric {
			.type = VK4::Vk_CameraType::Rasterizer_IM,
			.wPos = glm::tvec3<VK4::point_type> {5.0f, 5.0f, 5.0f },
			.wLook = glm::tvec3<VK4::point_type> {0.0f, 0.0f, 0.0f },
			.wUp = glm::tvec3<VK4::point_type> {0.0f, 0.0f, 1.0f },
			.fow = 80.0f / 180.0f * glm::pi<float>(),
			.wNear = 1.0f,
			.wFar = 100.0f,
			.steeringType = VK4::Vk_SteeringType::CAMERA_CENTRIC
		};

		VK4::Vk_RGBColor from { .r=0.5f, .g=0.0f, .b=0.0f };
		VK4::Vk_RGBColor to { .r=0.0f, .g=0.0f, .b=0.5f };

		VK4::Vk_GridLayout layout(3, 3, 0, 0);
		layout.vk_addCamera(0,0, {specs_ObjectCentric, VK4::Vk_ColorOp::rgb_lerp(0.000f, from, to)});
		layout.vk_addCamera(0,1, {specs_CameraCentric, VK4::Vk_ColorOp::rgb_lerp(0.125f, from, to)});
		layout.vk_addCamera(0,2, {specs_ObjectCentric, VK4::Vk_ColorOp::rgb_lerp(0.250f, from, to)});
		layout.vk_addCamera(1,0, {specs_CameraCentric, VK4::Vk_ColorOp::rgb_lerp(0.375f, from, to)});
		layout.vk_addCamera(1,1, {specs_ObjectCentric, VK4::Vk_ColorOp::rgb_lerp(0.500f, from, to)});
		layout.vk_addCamera(1,2, {specs_CameraCentric, VK4::Vk_ColorOp::rgb_lerp(0.625f, from, to)});
		layout.vk_addCamera(2,0, {specs_ObjectCentric, VK4::Vk_ColorOp::rgb_lerp(0.750f, from, to)});
		layout.vk_addCamera(2,1, {specs_CameraCentric, VK4::Vk_ColorOp::rgb_lerp(0.875f, from, to)});
		layout.vk_addCamera(2,2, {specs_ObjectCentric, VK4::Vk_ColorOp::rgb_lerp(1.000f, from, to)});

		cam.vk_addCamera(layout.vk_layoutList(width, height));

		auto cp = UT::Vk4TestData::Point_P();
		auto cc = UT::Vk4TestData::Point_C();
		auto ci = UT::Vk4TestData::Point_P_C_Indices();
		auto dot = VK4::S_Dot_P_C::create(
			&device,
			"test_object",
			glm::tmat4x4<VK4::point_type> {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1},
			cp, cc, ci, 2.0f, 1.0f
		);

        cam.vk_runThread();
		cam.vk_attachToAll(dot);
		cam.vk_rebuildAndRedraw();
        
		cam.vk_detachFromAll(dot);
	}
}

BOOST_AUTO_TEST_SUITE_END()