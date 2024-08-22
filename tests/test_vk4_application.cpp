#define BOOST_TEST_MODULE RunTestVk4Application
#include "boost/test/included/unit_test.hpp"

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

BOOST_AUTO_TEST_SUITE(RunTestVk4Application)

//std::unique_ptr<Vk_GlfwSurface> _surface;
//std::unique_ptr<Vk_Device> _device;
//std::unique_ptr<Vk_CommandPool> _commandPool;
//std::unique_ptr<Vk_SamplingResolution> _samplingResolution;

auto new_test = boost::unit_test::enabled();
auto all_tests = boost::unit_test::disabled();

BOOST_AUTO_TEST_CASE(Test_Instance, *all_tests) {
	std::unique_ptr<VK4::Vk_Instance> instance = std::make_unique<VK4::Vk_Instance>("test_app");
}

BOOST_AUTO_TEST_CASE(Test_Device, *all_tests) {
	std::string name = "test_app";

	std::unique_ptr<VK4::Vk_Instance> instance = std::make_unique<VK4::Vk_Instance>(name);
	std::unique_ptr<VK4::Vk_Device> device = std::make_unique<VK4::Vk_Device>(name);
	std::unique_ptr<VK4::Vk_Surface> surface = std::make_unique<VK4::Vk_Surface>(instance.get(), name, 500, 300, true);
}

template<class T_DataType, class ..._Types>
void testBuffer1(int trials, VK4::Vk_Device* const device, std::string name, const _Types&... test_config) {

	auto args = std::make_tuple(test_config...);
	auto data = std::get<0>(args).data(); // geometry.data();
	auto size = std::get<0>(args).size(); // geometry.size();
	auto updateBehaviour = std::get<1>(args); // VK4::Vk_BufferUpdateBehaviour::GlobalLock,  
	auto sizeBehaviour = std::get<2>(args); //   VK4::Vk_BufferSizeBehaviour::Init_1_0_Grow_1_5;

	std::unique_ptr<VK4::Vk_DataBuffer<T_DataType>> buffer
		= std::make_unique<VK4::Vk_DataBuffer<T_DataType>>(
			device, "TestObj", data, size, updateBehaviour, sizeBehaviour, name);

	// do unique checks
	assert(buffer->vk_bufferSize() == buffer->vk_maxBufferSize());
	assert(buffer->vk_bufferCount() == buffer->vk_maxBufferCount());

// #ifndef _DEBUG
// 	std::string identifier = buffer->identifier();
// 	std::chrono::microseconds duration = static_cast<std::chrono::microseconds>(0);
// 	std::chrono::microseconds duration2 = static_cast<std::chrono::microseconds>(0);
// 	for (int i = 0; i < trials; ++i) {
// 		auto start2 = std::chrono::high_resolution_clock::now();
// 		{
// 			auto start = std::chrono::high_resolution_clock::now();
// 			std::unique_ptr<VK4::Vk_DataBuffer<T_DataType>> buffer_ptest = std::make_unique<VK4::Vk_DataBuffer<T_DataType>>(device, data, size, behaviour, name);
// 			auto stop = std::chrono::high_resolution_clock::now();
// 			duration += std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
// 		}
// 		auto stop2 = std::chrono::high_resolution_clock::now();
// 		duration2 += std::chrono::duration_cast<std::chrono::microseconds>(stop2 - start2);
// 	}

// 	std::cout << UT::TestUtilities::formatTimeOutput(
// 		identifier + ": \n\tcreate={}, \n\tdestroy={}, \n\ttotal={}",
// 		duration.count() / trials,
// 		(duration2 - duration).count() / trials,
// 		duration2.count() / trials)
// 		<< std::endl;
// #endif
}

template<class T_DataType>
void resizeTestCases(VK4::Vk_DataBuffer<T_DataType>* buffer, size_t baseCount, std::vector<T_DataType>& data) {
	assert(buffer->vk_bufferCount() == buffer->vk_getRequiredInitCount(baseCount));
	assert(buffer->vk_maxBufferCount() == buffer->vk_getRequiredInitCount(baseCount));
	
	UT::TestUtilities::compareStructuresVectors<T_DataType>(buffer->vk_getData(), &data);
	
	buffer->vk_resize(static_cast<size_t>(std::ceil(2 * baseCount)));
	assert(buffer->vk_bufferCount() == buffer->vk_getRequiredInitCount(baseCount));
	assert(buffer->vk_maxBufferCount() == buffer->vk_getRequiredInitCount(2 * baseCount));

	UT::TestUtilities::compareStructuresVectors<T_DataType>(buffer->vk_getData(), &data);

	buffer->vk_resize(static_cast<size_t>(std::ceil(baseCount)));
	assert(buffer->vk_bufferCount() == buffer->vk_getRequiredInitCount(baseCount));
	assert(buffer->vk_maxBufferCount() == buffer->vk_getRequiredInitCount(baseCount));

	UT::TestUtilities::compareStructuresVectors<T_DataType>(buffer->vk_getData(), &data);
}

template<class T_DataType>
void testUpdateDataBuffer(VK4::Vk_DataBuffer<T_DataType>* buffer, std::vector<T_DataType>& data, size_t keep) {
	size_t newCount = buffer->vk_getNewRequiredMaxCount(data.size());
	buffer->vk_update(data.data(), data.size(), keep);
	assert(buffer->vk_maxBufferCount() == newCount);
	assert(buffer->vk_bufferCount() == data.size());
	UT::TestUtilities::compareStructuresVectors<T_DataType>(buffer->vk_getData(), &data);
}

template<class T_DataType>
void updateTestCases(VK4::Vk_DataBuffer<T_DataType>* buffer, std::vector<T_DataType>& data1) {
	size_t size1 = data1.size();

	size_t size2 = static_cast<size_t>(std::ceil(1.25 * size1));
	std::vector<T_DataType> tempData2 = UT::Vk4TestData::RandomLarge_Data<T_DataType>(size2 - size1, 2.0);
	std::vector<T_DataType> data2 = std::vector<T_DataType>(data1);
	data2.insert(data2.end(), tempData2.begin(), tempData2.end());
	testUpdateDataBuffer<T_DataType>(buffer, data2, size1);

	size_t size3 = static_cast<size_t>(std::ceil(1.5 * size1));
	std::vector<T_DataType> tempData3 = UT::Vk4TestData::RandomLarge_Data<T_DataType>(size3 - size2, 3.0);
	std::vector<T_DataType> data3 = std::vector<T_DataType>(data2);
	data3.insert(data3.end(), tempData3.begin(), tempData3.end());
	testUpdateDataBuffer<T_DataType>(buffer, data3, size2);

	size_t size4 = static_cast<size_t>(std::ceil(1.75 * size1));
	std::vector<T_DataType> tempData4 = UT::Vk4TestData::RandomLarge_Data<T_DataType>(size4 - size3, 4.0);
	std::vector<T_DataType> data4 = std::vector<T_DataType>(data3);
	data4.insert(data4.end(), tempData4.begin(), tempData4.end());
	testUpdateDataBuffer<T_DataType>(buffer, data4, size3);

	size_t size5 = static_cast<size_t>(std::ceil(2.0 * size1));
	std::vector<T_DataType> tempData5 = UT::Vk4TestData::RandomLarge_Data<T_DataType>(size5 - size4, 5.0);
	std::vector<T_DataType> data5 = std::vector<T_DataType>(data4);
	data5.insert(data5.end(), tempData5.begin(), tempData5.end());
	testUpdateDataBuffer<T_DataType>(buffer, data5, size4);

	size_t size6 = static_cast<size_t>(std::ceil(2.25 * size1));
	std::vector<T_DataType> tempData6 = UT::Vk4TestData::RandomLarge_Data<T_DataType>(size6 - size5, 6.0);
	std::vector<T_DataType> data6 = std::vector<T_DataType>(data5);
	data6.insert(data6.end(), tempData6.begin(), tempData6.end());
	testUpdateDataBuffer<T_DataType>(buffer, data6, size5);

	size_t size7 = static_cast<size_t>(std::ceil(3.1 * size1));
	std::vector<T_DataType> tempData7 = UT::Vk4TestData::RandomLarge_Data<T_DataType>(size7 - size6, 7.0);
	std::vector<T_DataType> data7 = std::vector<T_DataType>(data6);
	data7.insert(data7.end(), tempData7.begin(), tempData7.end());
	testUpdateDataBuffer<T_DataType>(buffer, data7, size6);

	size_t size8 = static_cast<size_t>(std::ceil(5.5 * size1));
	std::vector<T_DataType> tempData8 = UT::Vk4TestData::RandomLarge_Data<T_DataType>(size8 - size7, 8.0);
	std::vector<T_DataType> data8 = std::vector<T_DataType>(data7);
	data8.insert(data8.end(), tempData8.begin(), tempData8.end());
	testUpdateDataBuffer<T_DataType>(buffer, data8, size7);
}

BOOST_AUTO_TEST_CASE(Test_BufferResize2, *new_test) {
	std::string name = "test_app";

	std::unique_ptr<VK4::Vk_Instance> instance = std::make_unique<VK4::Vk_Instance>(name);
	std::unique_ptr<VK4::Vk_Surface> surface = std::make_unique<VK4::Vk_Surface>(instance.get(), name, 500, 300, true);
	std::unique_ptr<VK4::Vk_Device> device = std::make_unique<VK4::Vk_Device>(name);

	size_t testCount = static_cast<size_t>(std::ceil(std::pow(2, 10)));
	{
		// Vertices
		typedef VK4::Vk_Vertex_PC Type;
		auto data = UT::Vk4TestData::RandomLarge_Data<Type>(testCount, 1.0);
		std::unique_ptr<VK4::Vk_DataBuffer<Type>> buffer = std::make_unique<VK4::Vk_DataBuffer<Type>>(
			device.get(), "TestObj", data.data(), data.size(), VK4::Vk_BufferUpdateBehaviour::GlobalLock, VK4::Vk_BufferSizeBehaviour::Init_1_0_Grow_1_5, "TestBuffer");
		resizeTestCases<Type>(buffer.get(), testCount, data);
		updateTestCases<Type>(buffer.get(), data);
	}
	{
		// Vertices
		typedef VK4::Vk_Vertex_PCN Type;
		auto data = UT::Vk4TestData::RandomLarge_Data<Type>(testCount, 1.0);
		std::unique_ptr<VK4::Vk_DataBuffer<Type>> buffer = std::make_unique<VK4::Vk_DataBuffer<Type>>(
			device.get(), "TestObj", data.data(), data.size(), VK4::Vk_BufferUpdateBehaviour::GlobalLock, VK4::Vk_BufferSizeBehaviour::Init_1_0_Grow_2, "TestBuffer");

		resizeTestCases<Type>(buffer.get(), testCount, data);
		updateTestCases<Type>(buffer.get(), data);
	}
	{
		// Vertices
		typedef VK4::Vk_Vertex_PCNT Type;
		auto data = UT::Vk4TestData::RandomLarge_Data<Type>(testCount, 1.0);
		std::unique_ptr<VK4::Vk_DataBuffer<Type>> buffer = std::make_unique<VK4::Vk_DataBuffer<Type>>(
			device.get(), "TestObj", data.data(), data.size(), VK4::Vk_BufferUpdateBehaviour::GlobalLock, VK4::Vk_BufferSizeBehaviour::Init_1_5_Grow_1_5, "TestBuffer");
	
		resizeTestCases<Type>(buffer.get(), testCount, data);
		updateTestCases<Type>(buffer.get(), data);
	}
	{
		// Vertices
		typedef VK4::index_type Type;
		auto data = UT::Vk4TestData::RandomLarge_Data<Type>(testCount, 1.0);
		std::unique_ptr<VK4::Vk_DataBuffer<Type>> buffer = std::make_unique<VK4::Vk_DataBuffer<Type>>(
			device.get(), "TestObj", data.data(), data.size(), VK4::Vk_BufferUpdateBehaviour::GlobalLock, VK4::Vk_BufferSizeBehaviour::Init_1_5_Grow_2, "TestBuffer");
	
		resizeTestCases<Type>(buffer.get(), testCount, data);
		updateTestCases<Type>(buffer.get(), data);
	}
}

BOOST_AUTO_TEST_CASE(Test_BufferResize, *new_test) {
	std::string name = "test_app";

	std::unique_ptr<VK4::Vk_Instance> instance = std::make_unique<VK4::Vk_Instance>(name);
	std::unique_ptr<VK4::Vk_Surface> surface = std::make_unique<VK4::Vk_Surface>(instance.get(), name, 500, 300, true);
	std::unique_ptr<VK4::Vk_Device> device = std::make_unique<VK4::Vk_Device>(name);

	{
		// Vertices
		auto data = UT::Vk4TestData::Cube1_PC();
		typedef VK4::Vk_Vertex_PC Type;

		std::unique_ptr<VK4::Vk_DataBuffer<Type>> buffer = std::make_unique<VK4::Vk_DataBuffer<Type>>(
			device.get(), "TestObj", data.data(), data.size(), VK4::Vk_BufferUpdateBehaviour::GlobalLock, VK4::Vk_BufferSizeBehaviour::Init_1_0_Grow_1_5, "TestBuffer");

		assert(buffer->vk_cpuDataBufferCount() == 0);
		const std::vector<Type>* wbData = buffer->vk_getData();
		assert(wbData->size() == data.size());
		for (int i = 0; i < data.size(); ++i) { assert(Type::compare(wbData->at(i), data.at(i))); }

		device->printActiveDeviceMemoryProperties();

		double maxSize = static_cast<double>(buffer->vk_localBufferMemoryBudget());
		double dataSize = static_cast<double>(data.size() * sizeof(Type));
		size_t newSize1 = static_cast<size_t>(std::ceil(maxSize / 2) / sizeof(Type));
		buffer->vk_resize(newSize1);	

		const std::vector<Type>* wbData2 = buffer->vk_getData();
		assert(wbData2->size() == data.size());
		for (int i = 0; i < data.size(); ++i) { assert(Type::compare(wbData2->at(i), data.at(i))); }

		device->printActiveDeviceMemoryProperties();

		// this one should yield a buffer larger than half of the device local space
		// so, it should trigger the exception that will read back the data to main memory
		size_t newSize2 = static_cast<size_t>(std::ceil(0.75 * maxSize) / sizeof(Type));
		buffer->vk_resize(static_cast<size_t>(std::ceil(newSize2)));

		const std::vector<Type>* wbData3 = buffer->vk_getData();
		assert(wbData3->size() == data.size());
		for (int i = 0; i < data.size(); ++i) { assert(Type::compare(wbData3->at(i), data.at(i))); }
		
		device->printActiveDeviceMemoryProperties();

		buffer->vk_clearCpuBuffer();
		assert(buffer->vk_cpuDataBufferCount() == 0);
	}
}

BOOST_AUTO_TEST_CASE(Test_BuffersWriteback, *all_tests) {
	std::string name = "test_app";

	std::unique_ptr<VK4::Vk_Instance> instance = std::make_unique<VK4::Vk_Instance>(name);
	std::unique_ptr<VK4::Vk_Surface> surface = std::make_unique<VK4::Vk_Surface>(instance.get(), name, 500, 300, true);
	std::unique_ptr<VK4::Vk_Device> device = std::make_unique<VK4::Vk_Device>(name);

	{
		// Vertices
		auto data = UT::Vk4TestData::Cube1_PC();
		typedef VK4::Vk_Vertex_PC Type;

		std::unique_ptr<VK4::Vk_DataBuffer<Type>> buffer = std::make_unique<VK4::Vk_DataBuffer<Type>>(
			device.get(), "TestObj", data.data(), data.size(), VK4::Vk_BufferUpdateBehaviour::GlobalLock, VK4::Vk_BufferSizeBehaviour::Init_1_0_Grow_1_5, "TestBuffer");

		assert(buffer->vk_cpuDataBufferCount() == 0);
		const std::vector<Type>* wbData = buffer->vk_getData();

		assert(wbData->size() == data.size());
		for (int i = 0; i < data.size(); ++i) { assert(Type::compare(wbData->at(i), data.at(i))); }

		buffer->vk_clearCpuBuffer();
		assert(buffer->vk_cpuDataBufferCount() == 0);
	}
	
	{
		// Colors
		auto data = UT::Vk4TestData::Cube1_PCN();
		typedef VK4::Vk_Vertex_PCN Type;

		std::unique_ptr<VK4::Vk_DataBuffer<Type>> buffer = std::make_unique<VK4::Vk_DataBuffer<Type>>(
			device.get(), "TestObj", data.data(), data.size(), VK4::Vk_BufferUpdateBehaviour::GlobalLock, VK4::Vk_BufferSizeBehaviour::Init_1_0_Grow_1_5, "TestBuffer");

		assert(buffer->vk_cpuDataBufferCount() == 0);
		const std::vector<Type>* wbData = buffer->vk_getData();

		assert(wbData->size() == data.size());
		for (int i = 0; i < data.size(); ++i) { assert(Type::compare(wbData->at(i), data.at(i))); }

		buffer->vk_clearCpuBuffer();
		assert(buffer->vk_cpuDataBufferCount() == 0);
	}

	{
		// Normals
		auto data = UT::Vk4TestData::Cube1_PCNT();
		typedef VK4::Vk_Vertex_PCNT Type;

		std::unique_ptr<VK4::Vk_DataBuffer<Type>> buffer = std::make_unique<VK4::Vk_DataBuffer<Type>>(
			device.get(), "TestObj", data.data(), data.size(), VK4::Vk_BufferUpdateBehaviour::GlobalLock, VK4::Vk_BufferSizeBehaviour::Init_1_0_Grow_1_5, "TestBuffer");

		assert(buffer->vk_cpuDataBufferCount() == 0);
		const std::vector<Type>* wbData = buffer->vk_getData();

		assert(wbData->size() == data.size());
		for (int i = 0; i < data.size(); ++i) { assert(Type::compare(wbData->at(i), data.at(i))); }

		buffer->vk_clearCpuBuffer();
		assert(buffer->vk_cpuDataBufferCount() == 0);
	}

	{
		// Indices
		auto data = UT::Vk4TestData::Cube1_PCNT_Indices();
		typedef VK4::index_type Type;

		std::unique_ptr<VK4::Vk_DataBuffer<Type>> buffer = std::make_unique<VK4::Vk_DataBuffer<Type>>(
			device.get(), "TestObj", data.data(), data.size(), VK4::Vk_BufferUpdateBehaviour::GlobalLock, VK4::Vk_BufferSizeBehaviour::Init_1_0_Grow_1_5, "TestBuffer");

		assert(buffer->vk_cpuDataBufferCount() == 0);
		const std::vector<Type>* wbData = buffer->vk_getData();

		assert(wbData->size() == data.size());
		for (int i = 0; i < data.size(); ++i) { assert(wbData->at(i) == data.at(i)); }

		buffer->vk_clearCpuBuffer();
		assert(buffer->vk_cpuDataBufferCount() == 0);
	}
}

BOOST_AUTO_TEST_CASE(Test_Buffers, *all_tests) {
	std::string name = "test_app";

	std::unique_ptr<VK4::Vk_Instance> instance = std::make_unique<VK4::Vk_Instance>(name);
	std::unique_ptr<VK4::Vk_Surface> surface = std::make_unique<VK4::Vk_Surface>(instance.get(), name, 500, 300, true);
	std::unique_ptr<VK4::Vk_Device> device = std::make_unique<VK4::Vk_Device>(name);

	auto pc = UT::Vk4TestData::Cube1_PC();
	auto pcn = UT::Vk4TestData::Cube1_PCN();
	auto pcnt = UT::Vk4TestData::Cube1_PCNT();
	auto indices = UT::Vk4TestData::Cube1_PC_Indices();

	int trials = 100;
	
	testBuffer1<VK4::Vk_Vertex_PC>(trials, device.get(), "Testbuffer", pc,		VK4::Vk_BufferUpdateBehaviour::GlobalLock, VK4::Vk_BufferSizeBehaviour::Init_1_0_Grow_1_5);
	testBuffer1<VK4::Vk_Vertex_PCN>(trials, device.get(), "Testbuffer", pcn, 	VK4::Vk_BufferUpdateBehaviour::GlobalLock, VK4::Vk_BufferSizeBehaviour::Init_1_0_Grow_1_5);
	testBuffer1<VK4::Vk_Vertex_PCNT>(trials, device.get(), "Testbuffer", pcnt,	VK4::Vk_BufferUpdateBehaviour::GlobalLock, VK4::Vk_BufferSizeBehaviour::Init_1_0_Grow_1_5);
	testBuffer1<VK4::index_type>(trials, device.get(), "Testbuffer", indices,	VK4::Vk_BufferUpdateBehaviour::GlobalLock, VK4::Vk_BufferSizeBehaviour::Init_1_0_Grow_1_5);

	testBuffer1<VK4::Vk_Vertex_PC>(trials, device.get(), "Testbuffer", pc,		VK4::Vk_BufferUpdateBehaviour::GlobalLock, VK4::Vk_BufferSizeBehaviour::Init_1_0_Grow_1_5);
	testBuffer1<VK4::Vk_Vertex_PCN>(trials, device.get(), "Testbuffer", pcn,	VK4::Vk_BufferUpdateBehaviour::GlobalLock, VK4::Vk_BufferSizeBehaviour::Init_1_0_Grow_1_5);
	testBuffer1<VK4::Vk_Vertex_PCNT>(trials, device.get(), "Testbuffer", pcnt,	VK4::Vk_BufferUpdateBehaviour::GlobalLock, VK4::Vk_BufferSizeBehaviour::Init_1_0_Grow_1_5);
	testBuffer1<VK4::index_type>(trials, device.get(), "Testbuffer", indices,	VK4::Vk_BufferUpdateBehaviour::GlobalLock, VK4::Vk_BufferSizeBehaviour::Init_1_0_Grow_1_5);
}

BOOST_AUTO_TEST_CASE(Test_UniformBuffer, *all_tests) {
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

BOOST_AUTO_TEST_CASE(Test_ShaderLoader, *all_tests) {
	std::string name = "test_app";

	std::unique_ptr<VK4::Vk_Instance> instance = std::make_unique<VK4::Vk_Instance>(name);
	std::unique_ptr<VK4::Vk_Surface> surface = std::make_unique<VK4::Vk_Surface>(instance.get(), name, 500, 300, true);
	std::unique_ptr<VK4::Vk_Device> device = std::make_unique<VK4::Vk_Device>(name);

	{
		std::string fragShaderName = "shaderprograms/s_dot_pc.frag.spv";
		std::string vertShaderName = "shaderprograms/s_dot_pc.vert.spv";
		VK4::Vk_Shader vertShader(device.get(), vertShaderName);
		VK4::Vk_Shader fragShader(device.get(), fragShaderName);
	}
}

BOOST_AUTO_TEST_CASE(Test_Shader, *all_tests) {
	std::string name = VK4::S_Dot_P_C::Identifier;

	std::unique_ptr<VK4::Vk_Instance> instance = std::make_unique<VK4::Vk_Instance>(name);
	std::unique_ptr<VK4::Vk_Device> device = std::make_unique<VK4::Vk_Device>(name);
	std::unique_ptr<VK4::Vk_Surface> surface = std::make_unique<VK4::Vk_Surface>(instance.get(), name, 500, 300, true);
	std::unique_ptr<VK4::Vk_RenderPass_IM> renderpass = std::make_unique<VK4::Vk_RenderPass_IM>(device.get(), surface.get());
	std::unique_ptr<VK4::Vk_Swapchain_IM> swapchain = std::make_unique<VK4::Vk_Swapchain_IM>(device.get(), surface.get());
	std::unique_ptr<VK4::Vk_Framebuffer_IM> framebuffer = std::make_unique<VK4::Vk_Framebuffer_IM>(device.get(), surface.get(), swapchain.get(), renderpass.get());

	int camId = 0;
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

BOOST_AUTO_TEST_CASE(Test_Swapchain, *all_tests) {
	std::string name = "test_app";

	int camId = 0;
	{
		std::unique_ptr<VK4::Vk_Instance> instance = std::make_unique<VK4::Vk_Instance>(name);
		std::unique_ptr<VK4::Vk_Surface> surface = std::make_unique<VK4::Vk_Surface>(instance.get(), name, 500, 300, true);
		std::unique_ptr<VK4::Vk_Device> device = std::make_unique<VK4::Vk_Device>(name);
		std::unique_ptr<VK4::Vk_Swapchain_IM> swapchain = std::make_unique<VK4::Vk_Swapchain_IM>(device.get(), surface.get());
	}
	{
		std::unique_ptr<VK4::Vk_Instance> instance = std::make_unique<VK4::Vk_Instance>(name);
		std::unique_ptr<VK4::Vk_Surface> surface = std::make_unique<VK4::Vk_Surface>(instance.get(), name, 500, 300, true);
		std::unique_ptr<VK4::Vk_Device> device = std::make_unique<VK4::Vk_Device>(name);
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

BOOST_AUTO_TEST_CASE(Test_Renderpass, *all_tests) {
	std::string name = "test_app";

	int camId = 0;
	{
		std::unique_ptr<VK4::Vk_Instance> instance = std::make_unique<VK4::Vk_Instance>(name);
		std::unique_ptr<VK4::Vk_Surface> surface = std::make_unique<VK4::Vk_Surface>(instance.get(), name, 500, 300, true);
		std::unique_ptr<VK4::Vk_Device> device = std::make_unique<VK4::Vk_Device>(name);
		std::unique_ptr<VK4::Vk_RenderPass_IM> renderpass = std::make_unique<VK4::Vk_RenderPass_IM>(device.get(), surface.get());
	}
}

BOOST_AUTO_TEST_CASE(Test_Framebuffer, *all_tests) {
	std::string name = "test_app";

	{
		int camId = 0;
		std::unique_ptr<VK4::Vk_Instance> instance = std::make_unique<VK4::Vk_Instance>(name);
		std::unique_ptr<VK4::Vk_Device> device = std::make_unique<VK4::Vk_Device>(name);
		std::unique_ptr<VK4::Vk_Surface> surface = std::make_unique<VK4::Vk_Surface>(instance.get(), name, 500, 300, true);
		std::unique_ptr<VK4::Vk_RenderPass_IM> renderpass = std::make_unique<VK4::Vk_RenderPass_IM>(device.get(), surface.get());
		std::unique_ptr<VK4::Vk_Swapchain_IM> swapchain = std::make_unique<VK4::Vk_Swapchain_IM>(device.get(), surface.get());
		std::unique_ptr<VK4::Vk_Framebuffer_IM> framebuffer = std::make_unique<VK4::Vk_Framebuffer_IM>(device.get(), surface.get(), swapchain.get(), renderpass.get());
	}
}

BOOST_AUTO_TEST_CASE(Test_Graphicspipeline, *all_tests) {
	std::string name = "test_app";

	int camId = 0;
	{
		std::unique_ptr<VK4::Vk_Instance> instance = std::make_unique<VK4::Vk_Instance>(name);
		std::unique_ptr<VK4::Vk_Device> device = std::make_unique<VK4::Vk_Device>(name);
		std::unique_ptr<VK4::Vk_Surface> surface = std::make_unique<VK4::Vk_Surface>(instance.get(), name, 500, 300, true);
		std::unique_ptr<VK4::Vk_RenderPass_IM> renderpass = std::make_unique<VK4::Vk_RenderPass_IM>(device.get(), surface.get());
		std::unique_ptr<VK4::Vk_Swapchain_IM> swapchain = std::make_unique<VK4::Vk_Swapchain_IM>(device.get(), surface.get());
		std::unique_ptr<VK4::Vk_Framebuffer_IM> framebuffer = std::make_unique<VK4::Vk_Framebuffer_IM>(device.get(), surface.get(), swapchain.get(), renderpass.get());

		std::string name = "s_dot_p_c";
		std::string fragShaderName = "shaderprograms/" + name + ".frag.spv";
		std::string vertShaderName = "shaderprograms/" + name + ".vert.spv";
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

// BOOST_AUTO_TEST_CASE(Test_Rasterizer_IM, *all_tests) {
// 	std::string name = "test_app";
// 	//glfwInit();
// 	//glfwWi					// .margins = VK4::Vk_ViewportMargins{
// 					// 	.left = 50,
// 					// 	.right = 50,
// 					// 	.top = 50,
// 					// 	.bottom = 50
// 					// }	//GLFWwindow* window = glfwCreateWindow(500, 300, name.c_str(), nullptr, nullptr);

// 	int camId = 0;
// 	{
// 		std::unique_ptr<VK4::Vk_Device> device = std::make_unique<VK4::Vk_Device>(name);
// 		std::unique_ptr<VK4::Vk_Viewer> viewer = std::make_unique<VK4::Vk_Viewer>(device.get(), VK4::Vk_ViewerParams("name", 1200, 1024));

// 		std::unique_ptr<VK4::Vk_Rasterizer_IM> rasterizer = 
// 			std::make_unique<VK4::Vk_Rasterizer_IM>(
// 				camId,
// 				device.get(),
// 				VK4::I_Renderer::Vk_PipelineAuxilliaries{
// 					.surface=viewer->vk_surface(),
// 					.renderpass=viewer->vk_renderpass(),
// 					.swapchain=viewer->vk_swapchain(),
// 					.framebuffer=viewer->vk_framebuffer()
// 				},
// 				3,
// 				VK4::UniformBufferType_RendererMat4{ 
// 					.mat = glm::tmat4x4<VK4::point_type>(1.0f) 
// 				}
// 			);

// 		std::string objectName = "test_object";
// 		{
// 			auto cp = UT::Vk4TestData::Point_P();
// 			auto cc = UT::Vk4TestData::Point_C();
// 			auto ci = UT::Vk4TestData::Point_P_C_Indices();
// 			auto dot = VK4::S_Dot_P_C::create(
// 				device.get(),
// 				objectName,
// 				glm::tmat4x4<VK4::point_type> {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1},
// 				cp, cc, ci, 2.0f, 1.0f,
// 				// VK4::Topology::Points,
// 				VK4::CullMode::NoCulling
// 				// VK4::RenderType::Point
// 			);

// 			rasterizer->vk_attach(dot);
// 		}
// 		{
// 			auto obj = rasterizer->vk_detach(objectName);

// 			std::string name = VK4::S_Dot_P_C::Identifier;
// 			auto x1 = rasterizer->sizeofDescriptorPoolBuffer(name);
// 			auto x2 = rasterizer->sizeofDescriptorSetBuffer(name);
// 			auto x3 = rasterizer->sizeofLastDescriptorPool(name);
// 			auto x4 = rasterizer->sizeofAllDescriptorPools(name);
// 		}
// 	} 

// 	//glfwDestroyWindow(window);
// 	//glfwTerminate();
// }

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
			cp, cc, ci, 2.0f, 1.0f,
			VK4::CullMode::NoCulling
		);
		auto dot2 = VK4::S_Dot_P_C::create(
			&device,
			"test_object_2",
			glm::tmat4x4<VK4::point_type> {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1},
			cp, cc, ci, 2.0f, 1.0f,
			VK4::CullMode::NoCulling
		);

		cam.vk_attachToAll(dot);
		cam.vk_attachTo(0, dot2);
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
		VK4::Vk_Viewer cam(&device, VK4::Vk_ViewerParams("name", 1200, 1024));

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
						.r=0.5, .g=0.75, .b=0.1
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
						.r=0.37, .g=0.84, .b=0.21
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
			cp, cc, ci, 2.0f, 1.0f,
			VK4::CullMode::NoCulling
		);

		cam.vk_attachToAll(dot);
		cam.vk_build();
		cam.vk_run();
		cam.vk_detachFromAll(dot);
	}
}

BOOST_AUTO_TEST_CASE(Test_Camera_Scale, *all_tests) {
	std::string name = "lolol";
	{
		uint32_t width = 1800;
		uint32_t height = 1000;
		VK4::Vk_Device device(name);
		VK4::Vk_Viewer cam(&device, VK4::Vk_ViewerParams("name", 1200, 1024));

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
			cp, cc, ci, 2.0f, 1.0f,
			VK4::CullMode::NoCulling
		);

		cam.vk_attachToAll(dot);
		cam.vk_build();
		cam.vk_run();
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
		VK4::Vk_Viewer cam(&device, VK4::Vk_ViewerParams("name", 1200, 1024, VK4::Vk_ViewingType::GLOBAL));

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
			cp, cc, ci, 2.0f, 1.0f,
			VK4::CullMode::NoCulling
		);

		cam.vk_attachToAll(dot);
		cam.vk_build();
		cam.vk_run();
		cam.vk_detachFromAll(dot);
	}
}

BOOST_AUTO_TEST_CASE(Test_GridLayout_Local, *all_tests) {
	std::string name = "Test-GridLayout";
	{
		uint32_t width = 1400;
		uint32_t height = 1400;
		VK4::Vk_Device device(name);
		VK4::Vk_Viewer cam(&device, VK4::Vk_ViewerParams("name", 1200, 1024, VK4::Vk_ViewingType::LOCAL));

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
			cp, cc, ci, 2.0f, 1.0f,
			VK4::CullMode::NoCulling
		);

		cam.vk_attachToAll(dot);
		cam.vk_build();
		cam.vk_run();
		cam.vk_detachFromAll(dot);
	}
}

BOOST_AUTO_TEST_CASE(Test_GridLayout_Local_Irregular, *all_tests) {
	std::string name = "Test-GridLayout";
	{
		uint32_t width = 1024;
		uint32_t height = 800;
		VK4::Vk_Device device(name);
		VK4::Vk_Viewer cam(&device, VK4::Vk_ViewerParams("name", 1200, 1024, VK4::Vk_ViewingType::LOCAL));

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
			cp, cc, ci, 2.0f, 1.0f,
			VK4::CullMode::NoCulling
		);

		cam.vk_attachToAll(dot);
		cam.vk_build();
		cam.vk_run();
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
			cp, cc, ci, 2.0f, 1.0f,
			VK4::CullMode::NoCulling
		);

		cam.vk_attachToAll(dot);
		cam.vk_build();
		cam.vk_run();
		cam.vk_detachFromAll(dot);
	}
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

BOOST_AUTO_TEST_CASE(Test_FunctionWrapper, *all_tests) {
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

// class [[deprecated]] Viewer {
// 	public:
// 	Viewer() : _angle(0.0f), _size(1.0f), _step(0.1f), _on(false) {
// 		std::string name = "Viewer";
// 		uint32_t width = 1024;
// 		uint32_t height = 800;
// 		_device = std::make_unique<VK4::Vk_Device>(name);
// 		_cam = std::make_unique<VK4::Vk_Viewer>(_device.get(), VK4::Vk_ViewerParams("name", width, height, VK4::Vk_ViewingType::LOCAL, "."));

// 		VK4::Vk_CameraSpecs specs_ObjectCentric {
// 			.type = VK4::Vk_CameraType::Rasterizer_IM,
// 			.wPos = glm::tvec3<VK4::point_type> {5.0f, 5.0f, 5.0f },
// 			.wLook = glm::tvec3<VK4::point_type> {0.0f, 0.0f, 0.0f },
// 			.wUp = glm::tvec3<VK4::point_type> {0.0f, 0.0f, 1.0f },
// 			.fow = 80.0f / 180.0f * glm::pi<float>(),
// 			.wNear = 1.0f,
// 			.wFar = 100.0f,
// 			.steeringType = VK4::Vk_SteeringType::OBJECT_CENTRIC
// 		};

// 		VK4::Vk_CameraSpecs specs_CameraCentric {
// 			.type = VK4::Vk_CameraType::Rasterizer_IM,
// 			.wPos = glm::tvec3<VK4::point_type> {5.0f, 5.0f, 5.0f },
// 			.wLook = glm::tvec3<VK4::point_type> {0.0f, 0.0f, 0.0f },
// 			.wUp = glm::tvec3<VK4::point_type> {0.0f, 0.0f, 1.0f },
// 			.fow = 80.0f / 180.0f * glm::pi<float>(),
// 			.wNear = 1.0f,
// 			.wFar = 100.0f,
// 			.steeringType = VK4::Vk_SteeringType::CAMERA_CENTRIC
// 		};

// 		VK4::Vk_RGBColor from { .r=0.5f, .g=0.0f, .b=0.0f };
// 		VK4::Vk_RGBColor to { .r=0.0f, .g=0.0f, .b=0.5f };

// 		VK4::Vk_GridLayout layout(3, 3, 0, 0);
// 		layout.vk_addCamera(0,0, {specs_ObjectCentric, VK4::Vk_ColorOp::rgb_lerp(0.000f, from, to)});
// 		layout.vk_addCamera(0,1, {specs_CameraCentric, VK4::Vk_ColorOp::rgb_lerp(0.125f, from, to)});
// 		layout.vk_addCamera(0,2, {specs_ObjectCentric, VK4::Vk_ColorOp::rgb_lerp(0.250f, from, to)});
// 		layout.vk_addCamera(1,0, {specs_CameraCentric, VK4::Vk_ColorOp::rgb_lerp(0.375f, from, to)});
// 		layout.vk_addCamera(1,1, {specs_ObjectCentric, VK4::Vk_ColorOp::rgb_lerp(0.500f, from, to)});
// 		layout.vk_addCamera(1,2, {specs_CameraCentric, VK4::Vk_ColorOp::rgb_lerp(0.625f, from, to)});
// 		layout.vk_addCamera(2,0, {specs_ObjectCentric, VK4::Vk_ColorOp::rgb_lerp(0.750f, from, to)});
// 		layout.vk_addCamera(2,1, {specs_CameraCentric, VK4::Vk_ColorOp::rgb_lerp(0.875f, from, to)});
// 		layout.vk_addCamera(2,2, {specs_ObjectCentric, VK4::Vk_ColorOp::rgb_lerp(1.000f, from, to)});

// 		_cam->vk_addCamera(layout.vk_layoutList(width, height));

// 		_pointSize = 10.0f;
// 		_lineWidth = 5.0f;
// 		_alpha = 0.5f;

// 		auto cp = UT::Vk4TestData::Point_P();
// 		auto cc = UT::Vk4TestData::Point_C();
// 		auto ci = UT::Vk4TestData::Point_P_C_Indices();
// 		_dot = VK4::S_Dot_P_C::create(
// 			_device.get(),
// 			"test_object",
// 			glm::tmat4x4<VK4::point_type> {1,0,0,0, 0,1,0,0, 0,0,1,0, 1.5,1.5,0,1},
// 			cp, cc, ci, _pointSize, _alpha,
// 			VK4::CullMode::NoCulling
// 		);

// 		_line = VK4::S_Line_P_C::create(
// 			_device.get(),
// 			"test_line_obj",
// 			glm::tmat4x4<VK4::point_type> {1,0,0,0, 0,1,0,0, 0,0,1,0, -1.5,1.5,0,1},
// 			UT::Vk4TestData::Line_P(), UT::Vk4TestData::Line_C(), UT::Vk4TestData::Line_P_C_Indices(),
// 			_lineWidth, 1.0f,
// 			VK4::CullMode::NoCulling
// 		);

// 		float f=-4.0f;
// 		float t= 4.0f;
// 		float l= 0.5f;
// 		_coords = VK4::S_Line_P_C::create(
// 			_device.get(),
// 			"coords",
// 			glm::tmat4x4<VK4::point_type> {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1},
// 			UT::Vk4TestData::Coords_P(f,t,l, f,t,l, f,t,l), 
// 			UT::Vk4TestData::Coords_C(1.0f, 1.0f, 1.0f), 
// 			UT::Vk4TestData::Coords_P_C_Indices(),
// 			2.0f, 1.0f,
// 			VK4::CullMode::NoCulling
// 		);

// 		_mesh = VK4::S_Mesh_P_C::create(
// 			_device.get(),
// 			"mesh",
// 			glm::tmat4x4<VK4::point_type> {1,0,0,0, 0,1,0,0, 0,0,1,0, -1.5,-1.5,0,1},
// 			UT::Vk4TestData::Cube1_P(),
// 			UT::Vk4TestData::Cube1_C(),
// 			UT::Vk4TestData::Cube1_P_C_Indices(),
// 			1.0f,
// 			VK4::CullMode::Back,
// 			VK4::RenderType::Solid,
// 			1.0f, 1.0f
// 		);

// 		_mesh2 = VK4::S_Mesh_P_C::create(
// 			_device.get(),
// 			"mesh2",
// 			glm::tmat4x4<VK4::point_type> {1,0,0,0, 0,1,0,0, 0,0,1,0, 1.5,-1.5,0,1},
// 			UT::Vk4TestData::Cube2_P(_angle),
// 			UT::Vk4TestData::Cube2_C(),
// 			UT::Vk4TestData::Cube2_P_C_N_Indices(),
// 			1.0f,
// 			VK4::CullMode::Back,
// 			VK4::RenderType::Wireframe,
// 			1.0f, 1.0f
// 		);

// 		_mesh2Normals = VK4::S_Line_P_C::create(
// 			_device.get(),
// 			"mesh2_normals",
// 			glm::tmat4x4<VK4::point_type> {1,0,0,0, 0,1,0,0, 0,0,1,0, 1.5,-1.5,0,1},
// 			UT::Vk4TestData::Cube2_NormalLines_P(0.5f, _angle), UT::Vk4TestData::Cube2_NormalLines_C(), UT::Vk4TestData::Cube2_NormalLines_Indices(),
// 			_lineWidth, 1.0f,
// 			VK4::CullMode::NoCulling
// 		);

// 		_cam->vk_registerAction('r', this, &Viewer::rotate);
// 		_cam->vk_registerAction('s', this, &Viewer::scale);
// 		_cam->vk_registerAction('o', this, &Viewer::onoff);
// 		_cam->vk_registerAction('i', this, &Viewer::incr_p_size);
// 		_cam->vk_registerAction('d', this, &Viewer::decr_p_size);
// 		_cam->vk_registerAction('z', this, &Viewer::decr_alpha);
// 		_cam->vk_registerAction('x', this, &Viewer::incr_alpha);
// 		_cam->vk_registerAction('1', this, &Viewer::decr_l_width);
// 		_cam->vk_registerAction('2', this, &Viewer::incr_l_width);

// 		_cam->vk_attachToAll(_dot);
// 		_cam->vk_attachToAll(_line);
// 		_cam->vk_attachToAll(_coords);
// 		_cam->vk_attachToAll(_mesh);
// 		_cam->vk_attachToAll(_mesh2);
// 		_cam->vk_attachToAll(_mesh2Normals);
// 		_cam->vk_build();
// 		_cam->vk_run();
// 		_cam->vk_detachFromAll(_dot);
// 		_cam->vk_detachFromAll(_line);
// 	}

// private:
// 	float _angle;
// 	float _size;
// 	float _step;
// 	float _pointSize;
// 	float _lineWidth;
// 	float _alpha;
// 	bool _on;
// 	std::unique_ptr<VK4::Vk_Device> _device;
// 	std::unique_ptr<VK4::Vk_Viewer> _cam;
// 	std::shared_ptr<VK4::Vk_Dot<VK4::ObjectType_P_C>> _dot;
// 	std::shared_ptr<VK4::Vk_Line<VK4::ObjectType_P_C>> _line;
// 	std::shared_ptr<VK4::Vk_Line<VK4::ObjectType_P_C>> _coords;
// 	std::shared_ptr<VK4::Vk_Mesh<VK4::ObjectType_P_C>> _mesh;

// 	std::shared_ptr<VK4::Vk_Mesh<VK4::ObjectType_P_C>> _mesh2;
// 	std::shared_ptr<VK4::Vk_Line<VK4::ObjectType_P_C>> _mesh2Normals;

// 	void onoff(std::function<void()> repeat) {
// 		_on = !_on;
// 	}

// 	void incr_l_width(std::function<void()> repeat){
// 		_lineWidth++;
// 		_line->vk_updateLineWidth(_lineWidth);
// 		_cam->vk_rebuildAndRedraw();
// 	}

// 	void decr_l_width(std::function<void()> repeat){
// 		_lineWidth--;
// 		if(_lineWidth < 1.0f) _lineWidth = 1.0f;
// 		_line->vk_updateLineWidth(_lineWidth);
// 		_cam->vk_rebuildAndRedraw();
// 	}

// 	void incr_p_size(std::function<void()> repeat){
// 		_pointSize++;
// 		_dot->vk_updatePointSize(_pointSize);
// 		_cam->vk_rebuildAndRedraw();
// 	}

// 	void decr_p_size(std::function<void()> repeat){
// 		_pointSize--;
// 		if(_pointSize < 1.0f) _pointSize = 1.0f;
// 		_dot->vk_updatePointSize(_pointSize);
// 		_cam->vk_rebuildAndRedraw();
// 	}

// 	void decr_alpha(std::function<void()> repeat){
// 		_alpha -= 0.1f;
// 		if(_alpha < 0.0f) _alpha = 0.0f;
// 		_dot->vk_updateAlpha(_alpha);
// 		_cam->vk_rebuildAndRedraw();
// 	}

// 	void incr_alpha(std::function<void()> repeat){
// 		_alpha += 0.1f;
// 		if(_alpha > 1.0f) _alpha = 1.0f;
// 		_dot->vk_updateAlpha(_alpha);
// 		_cam->vk_rebuildAndRedraw();
// 	}

// 	void rotate(std::function<void()> repeat){
// 		_angle += 1.0f;
// 		if(_angle >= 360.0f){
// 			_angle = _angle - 360.0f;
// 		}

// 		_dot->vk_updatePoints(UT::Vk4TestData::Point_P(_angle / 180.0f * M_PI), 0);
// 		_cam->vk_rebuildAndRedraw();
// 		std::this_thread::sleep_for(std::chrono::microseconds(5000));
		
// 		if(_on) repeat();
// 	}

// 	void scale(std::function<void()> repeat) {
// 		if(_size >= 1.5f) _step = -0.01f;
// 		else if(_size <= 0.5f) _step = 0.01f;
// 		_size += _step;
// 		_dot->vk_updateModelMatrix(glm::tmat4x4<VK4::point_type>{_size,0,0,0, 0,_size,0,0, 0,0,_size,0, 0,0,0,1});
// 		std::this_thread::sleep_for(std::chrono::microseconds(5000));

// 		if(_on) repeat();
// 	}
// };

// BOOST_AUTO_TEST_CASE(Test_RegisterFunction, *boost::unit_test::disabled()) {
// 	{
// 		Viewer v;
// 	}
// }

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

BOOST_AUTO_TEST_CASE(Test_RandomIterator, *all_tests) {
	{
		IterableClass vv({{0, "zero"}, {1, "one"}, {2, "two"}});
		for(const auto& v : vv){
			std::cout << v << std::endl;
		}
	}
}

BOOST_AUTO_TEST_SUITE_END()