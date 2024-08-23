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

BOOST_AUTO_TEST_SUITE(RunTestVk4Application)

auto new_test = boost::unit_test::enabled();
auto all_tests = boost::unit_test::disabled();

BOOST_AUTO_TEST_CASE(Test_Instance, *new_test) {
	std::unique_ptr<VK4::Vk_Instance> instance = std::make_unique<VK4::Vk_Instance>("test_app");
}

BOOST_AUTO_TEST_CASE(Test_Device, *new_test) {
	std::string name = "test_app";

	std::unique_ptr<VK4::Vk_Instance> instance = std::make_unique<VK4::Vk_Instance>(name);
	std::unique_ptr<VK4::Vk_Device> device = std::make_unique<VK4::Vk_Device>(name);
	std::unique_ptr<VK4::Vk_Surface> surface = std::make_unique<VK4::Vk_Surface>(instance.get(), name, 500, 300, true);
}

class CamSim {
public:
	CamSim() : cam1(1), cam2(2) {}
	int cam1;
	int cam2; 
};

class TestObj {
public:
	TestObj() : y(0), counter(0), run('n') {}

	void f0(std::function<void()> repeat) {
		y++;
	}

	void f1(std::function<void()> repeat, int& x){
		x = x+1;
	}

	void inc1(std::function<void()> repeat, CamSim& sim){
		sim.cam1++;
	}

	void inc2(std::function<void()> repeat, CamSim& sim){
		sim.cam2++;
	}

	void repeater(std::function<void()> repeat) {
		repeat();
	}

	void onoff(std::function<void()> repeat) {
		if(run == 'n') run = 'y';
		else run = 'n';
		std::cout << "Change run to " << run << std::endl;
	}

	void test(std::function<void()> repeat) {
		std::cout << "test " << counter << std::endl;
		counter++;
		if(run == 'y'){
			repeat();
		}
	}

	int y;
	int counter;
	char run;
};

BOOST_AUTO_TEST_CASE(Test_FunctionWrapper, *new_test) {
	CamSim sim;
	TestObj t;

	std::queue<std::pair<std::shared_ptr<VK4::Vk_Func>, std::function<void()>>> jobs;
	{
		jobs.push(
			std::pair<std::shared_ptr<VK4::Vk_Func>, std::function<void()>>(
				VK4::Vk_TFunc(&t, &TestObj::inc1, { sim }).get(), 
				[](){
					std::cout << "do whatever" << std::endl; 
				}
			)
		);

		jobs.push(
			std::pair<std::shared_ptr<VK4::Vk_Func>, std::function<void()>>(
				VK4::Vk_TFunc(&t, &TestObj::inc2, { sim }).get(), 
				[](){
					std::cout << "do whatever" << std::endl; 
				}
			)
		);

		jobs.push(
			{
				VK4::Vk_TFunc(&t, &TestObj::f1, { sim.cam1 }).get(), 
				[](){
					std::cout << "do whatever" << std::endl; 
				}
			}
		);

		jobs.push(
			{
				VK4::Vk_TFunc(&t, &TestObj::f0, {}).get(), 
				[](){}
			}
		);

		jobs.push(
			{
				VK4::Vk_TFunc(&t, &TestObj::repeater, {}).get(), 
				[](){}
			}
		);
	}

	auto var0 = jobs.front();
	jobs.pop();
	(*var0.first)([](){  });
	BOOST_ASSERT(sim.cam1 == 2);
	var0.second();

	auto var1 = jobs.front();
	jobs.pop();
	(*var1.first)([](){  });
	BOOST_ASSERT(sim.cam2 == 3);
	var1.second();

	auto var2 = jobs.front();
	jobs.pop();
	(*var2.first)([](){  });
	BOOST_ASSERT(sim.cam1 == 3);
	var2.second();

	auto var3 = jobs.front();
	jobs.pop();
	(*var3.first)([](){  });
	BOOST_ASSERT(t.y == 1);
	var3.second();

	auto var4 = jobs.front();
	jobs.pop();
	(*var4.first)(
		[&t](){
			t.counter++;
		});
	BOOST_ASSERT(t.counter == 1);
	var3.second();
}

class IterableClass
{
private:
    std::map<int, std::string> m_internalContainer;
public:
    struct Iterator
    {
        using iterator_category = std::forward_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = std::string;
        using pointer = std::string*;
        using reference = std::string&;

        Iterator(std::map<int, std::string>::iterator ptr) : m_ptr(ptr) {}

        reference operator*() const { return m_ptr->second; }
        pointer operator->() { return &m_ptr->second; }
        Iterator& operator++() { m_ptr++; return *this; }
        Iterator operator++(int) { Iterator tmp = *this; ++(*this); return tmp; }
        friend bool operator== (const Iterator& a, const Iterator& b) { return a.m_ptr == b.m_ptr; };
        friend bool operator!= (const Iterator& a, const Iterator& b) { return a.m_ptr != b.m_ptr; };

    private:
        std::map<int, std::string>::iterator m_ptr;
    };

	IterableClass(std::map<int, std::string> init): m_internalContainer(init) {}

	Iterator begin() {return Iterator(m_internalContainer.begin()); }
	Iterator end() {return Iterator(m_internalContainer.end());}
};

BOOST_AUTO_TEST_CASE(Test_RandomIterator, *new_test) {
	{
		IterableClass vv({{0, "zero"}, {1, "one"}, {2, "two"}});
		for(const auto& v : vv){
			std::cout << v << std::endl;
		}
	}
}

BOOST_AUTO_TEST_SUITE_END()