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

#include "../src/Vk_Viewer.hpp"
#include "../src/Vk_SampleObjects.hpp"

#define TEST_SCENARIO_1
#include "test_data.hpp"
#undef TEST_SCENARIO_1
#include "test_utilities.hpp"

BOOST_AUTO_TEST_SUITE(RunTestVk4BufferUpdateStrategies)

auto new_test = boost::unit_test::enabled();
auto all_tests = boost::unit_test::disabled();

class TestViewer {
	public:
	TestViewer(
		bool manual,
		VK4::point_type minVal, 
		VK4::point_type maxVal, 
		size_t size, 
		VK4::Vk_BufferUpdateBehaviour updateBehaviour
	) 
	: _size(size), _minVal(minVal), _maxVal(maxVal), _on(false)
	{
		std::string name = "TestViewer";
		uint32_t width = 1024;
		uint32_t height = 800;
		_device = std::make_unique<VK4::Vk_Device>(name, VK4::Vk_DevicePreference::USE_DISCRETE_GPU);
		_cam = std::make_unique<VK4::Vk_Viewer>(_device.get(), VK4::Vk_ViewerParams(name, width, height, VK4::Vk_ViewingType::LOCAL, "."));

		VK4::Vk_CameraSpecs specs_ObjectCentric {
			.type = VK4::Vk_CameraType::Rasterizer_IM,
			.wPos = glm::tvec3<VK4::point_type> {2*_maxVal, 2*_maxVal, 2*_maxVal },
			.wLook = glm::tvec3<VK4::point_type> {0.0f, 0.0f, 0.0f },
			.wUp = glm::tvec3<VK4::point_type> {0.0f, 0.0f, 1.0f },
			.fow = 80.0f / 180.0f * glm::pi<float>(),
			.wNear = 0.001f,
			.wFar = 100.0f,
			.steeringType = VK4::Vk_SteeringType::OBJECT_CENTRIC
		};

		VK4::Vk_GridLayout layout(1, 1, 0, 0);
		layout.vk_addCamera(0,0, {specs_ObjectCentric, VK4::Vk_RGBColor{ .r=1.0f, .g=1.0f, .b=1.0f }});
		_cam->vk_addCamera(layout.vk_layoutList(width, height));

		float pointSize = 5.0f;
		float alpha = 0.5f;

		auto cp = VK4::Vk_SampleObjects::UniformRandom_PointObjData<VK4::Vk_Vertex_P>(_size, _minVal, _maxVal);
		auto cc = VK4::Vk_SampleObjects::UniformRandom_PointObjData<VK4::Vk_Vertex_C>(_size, 0, 0);
		auto ci = VK4::Vk_SampleObjects::UniformRandom_PointObjData<VK4::index_type>(_size, 0, 0);
		_dot = VK4::S_Dot_P_C::create(
			_device.get(), "test_object", glm::tmat4x4<VK4::point_type> {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1},
			cp, cc, ci, pointSize, alpha, updateBehaviour
		);

		float f=-4.0f;
		float t= 4.0f;
		float l= 0.5f;
		_coords = VK4::S_Line_P_C::create(
			_device.get(), "coords", glm::tmat4x4<VK4::point_type> {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1},
			VK4::Vk_SampleObjects::Coords_P(f,t,l, f,t,l, f,t,l),
			VK4::Vk_SampleObjects::Coords_C(1.0f, 1.0f, 1.0f), 
			VK4::Vk_SampleObjects::Coords_P_C_Indices(),
			2.0f, 1.0f
		);

		_cam->vk_registerAction('o', this, &TestViewer::onoff);
		_cam->vk_registerAction('u', this, &TestViewer::update);
		_cam->vk_runThread();

		_cam->vk_attachToAll(_dot);
		_cam->vk_attachToAll(_coords);
		// _cam->vk_rebuildAndRedraw();

		// defered
		// immediate

		/* empty space to do stuff */
		if(manual){
			int counter = 0;
			while(_cam->vk_running()){
				std::this_thread::sleep_for(std::chrono::milliseconds(500));
				std::cout << "hello world: " << counter++ << std::endl;
			}

			std::cout << "terminate at:" << counter << std::endl;
		}
		else{
			_cam->vk_stopThread();
		}

		_cam->vk_detachFromAll(_dot);
		_cam->vk_detachFromAll(_coords);
	}

private:
	std::unique_ptr<VK4::Vk_Device> _device;
	std::unique_ptr<VK4::Vk_Viewer> _cam;
	std::shared_ptr<VK4::Vk_Dot<VK4::ObjectType_P_C>> _dot;
	std::shared_ptr<VK4::Vk_Line<VK4::ObjectType_P_C>> _coords;

	size_t _size;
	VK4::point_type _minVal;
	VK4::point_type _maxVal;
	bool _on;

	void onoff(std::function<void()> repeat) {
		_on = !_on;
		std::cout << " ==================================== onoff ====================================: " << static_cast<int>(_on) << std::endl;
	}

	void update(std::function<void()> repeat){
		auto cp = VK4::Vk_SampleObjects::UniformRandom_PointObjData<VK4::Vk_Vertex_P>(_size, _minVal, _maxVal);
		_dot->vk_updatePoints(cp, 0, 0, VK4::Vk_ObjUpdate::Promptly);
		// _cam->vk_rebuildAndRedraw();
		std::cout << "update..." << std::endl;
		if(_on) repeat();
	}
};

BOOST_AUTO_TEST_CASE(Test_X, *new_test) {
	// deadlock somewhere
	TestViewer v(true, -1.0f, 1.0f, 1000, VK4::Vk_BufferUpdateBehaviour::Staged_DoubleBuffering);
}

BOOST_AUTO_TEST_SUITE_END()