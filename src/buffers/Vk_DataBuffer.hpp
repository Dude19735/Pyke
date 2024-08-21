#pragma once

#include <mutex>
#include <typeinfo>

#include "../Defines.h"
// #include "../Vk_Logger.hpp"
#include "../application/Vk_Device.hpp"
#include "../objects/Vk_Structures.hpp"
#include "Vk_DataBufferLib.hpp"

namespace VK4 {

	template<typename T_StructureType>
	class Vk_DataBuffer {
	public:

		Vk_DataBuffer(
			Vk_Device* const device,
			const std::string& associatedObject,
			const T_StructureType* structuredData,
			size_t count,
			Vk_BufferUpdateBehaviour updateBehaviour,
			Vk_BufferSizeBehaviour sizeBehaviour,
			std::string objName = ""
		)
			:
			_device(device),
			_cpuDataBuffer({}),
			_count(count),
			_maxCount(Vk_DataBufferLib::getInitMaxCount(count, sizeBehaviour)),
			_objName(objName + "[" + std::string(typeid(T_StructureType).name()) + "]"),
			_sizeBehaviour(sizeBehaviour),
			_updateBehaviour(updateBehaviour),
			_type(Vk_DataBufferLib::getInitBufferType<T_StructureType>()),
			_associatedObject("(=" + associatedObject + "=)"),
			_bufferIndex(0),
			_buffer({}),
			_bufferMemory({})
		{
			Vk_Logger::Log(typeid(this), GlobalCasters::castConstructorTitle(
				Vk_Lib::formatWithObjName(_objName, (std::string("Create Vertex Buffer ") + std::string(typeid(T_StructureType).name()) + _associatedObject))));

			Vk_DataBufferLib::checkAsserts(_objName, vk_bufferCount());
			createDataBufferForUpdateStrategy(structuredData);
		}

		~Vk_DataBuffer() {
			Vk_Logger::Log(typeid(this), GlobalCasters::castDestructorTitle(
				Vk_Lib::formatWithObjName(_objName, (std::string("Destroy Vertex Buffer ") + std::string(typeid(T_StructureType).name()) + _associatedObject))));

			for(int i=0; i<_buffer.size(); ++i){
				Vk_DataBufferLib::destroyGpuBuffer(_device, _buffer.at(i), _bufferMemory.at(i));
			}
		}

		/*
		* Get the Vulkan buffer descriptor
		*/
		VkBuffer vk_buffer() {
			return _buffer.at(_bufferIndex);
		}

		/*
		* Get the currently occupied size of the buffer in bytes
		*/
		size_t vk_bufferSize() {
			return static_cast<size_t>(_count * sizeof(T_StructureType));
		}

		/*
		* Get the current max buffer size in bytes
		*/
		size_t vk_maxBufferSize() {
			size_t s = sizeof(T_StructureType);
			return static_cast<size_t>(_maxCount * s);
		}

		/*
		* Get the current count of elements currently in the buffer
		*/
		size_t vk_bufferCount() {
			return _count;
		}

		/*
		* Get the current max amount of elements the buffer can hold before resizing
		*/
		size_t vk_maxBufferCount() {
			return _maxCount;
		}

		/*
		* Resize the buffer to hold a max of newCount
		*/
		void vk_resize(size_t newCount) {
			auto lock = std::lock_guard<std::mutex>(_localMutex);
			resizeDataBufferForUpdateStrategy(newCount, _count);
		}

		/*
		* Get the max amount of elements the buffer will be able to hold after the
		* next resize according to the resize strategy of the buffer
		*/
		size_t vk_nextBufferMaxCount(size_t count) {
			return getNextMaxCount(count);
		}

		/*
		* Return a string holding the object name the buffer is holding, the type and resize
		* strategy
		*/
		std::string vk_toString() {
			return
				std::string("Data buffer ")
				+ _objName
				+ std::string(", ")
				+ Vk_DataBufferLib::BufferTypeToString(_type)
				+ std::string(", ")
				+ Vk_BufferSizeBehaviourToString(_sizeBehaviour);
		}

		/*
		* Write the data of the GPU buffer into a CPU size vector and return a pointer
		* to the CPU sized buffer. Call vk_clearCpuBuffer() to clear the CPU sized data vector.
		*/
		const std::vector<T_StructureType>* vk_getData() {
			getDataToCpu();
			return &_cpuDataBuffer;
		}
		
		/*
		* Clear the CPU sided vector. Note: this will not clear the GPU sided memory.
		*/
		void vk_clearCpuBuffer() {
			_cpuDataBuffer.clear();
		}

		/*
		* Return the size of the CPU sided data vector. Note: this does not necessairily reflect
		* the size of the GPU sided data.
		*/
		size_t vk_cpuDataBufferCount() {
			return _cpuDataBuffer.size();
		}

		/*
		* Return the total available GPU local memory. This accesses the device and returns
		* the value the device returns.
		*/
		std::uint64_t vk_localBufferMemoryBudget() {
			return Vk_DataBufferLib::getDeviceLocalBufferMaxMemory(_device);
		}

		/*
		* Return the total available GPU/CPU shared memory. In general this is GPU main memory that
		* can be used by the GPU and usually corresponds to half of the computer main memory. This accesses
		* the device and returns the value the device returns.
		*/
		std::uint64_t vk_stagingBufferMemoryBudget() {
			return Vk_DataBufferLib::getStagingBufferMaxMemory(_device);
		}

		/*
		* Return the element count the buffer will be initialized with given the parameter count
		* according to the buffer's initialization strategy.
		*/
		size_t vk_getRequiredInitCount(size_t count) {
			return Vk_DataBufferLib::getInitMaxCount(count, _sizeBehaviour);
		}


		/*
		* Get the new max buffer count given a hypothetical newCount based on the 
		* buffer's resize strategy.
		*/
		size_t vk_getNewRequiredMaxCount(size_t newCount) {
			size_t newMaxCount;
			Vk_DataBufferLib::getNewBufferCount(_sizeBehaviour, _maxCount, newCount, newMaxCount);
			return newMaxCount;
		}

		/*
		* Update the buffer with the new structuredData of size newCount from the offset
		* newFrom on. This will update the data in the buffer starting from the newFrom offset up
		* to newTo offset.
		* Both newFrom and newTo must be smaller than newCount. newCount is the **total** count of the
		* new buffer data. **count** is the number of elements, not the byte size.
		* If newTo=-1 it will be set to newCount and everything starting from newFrom will be updated.
		* Semantic behaviour:
		* ===================
		*  * newCount > current count: 
		*     - buffer will be resized and the interval [newFrom, newTo) will be copied into it
		*     - the programmer is responsible for making sure that the transfer makes sense.
		*     - examples:
		*        # newCount = oldCount + X, newFrom = oldCount, newTo = oldCount + X
		*        # newCount = oldCount, newFrom = X1 > 0, newTo = X2 < newCount
		*        # newCount = oldCount - X, newFrom = 0, newTo = newCount
		*/
		void vk_update(
			const T_StructureType* structuredData,
			size_t newCount,
			size_t newFrom,
			size_t newTo=-1
		) {
			updateDataBufferForUpdateStrategy(structuredData, newCount, newFrom, newTo);
		}

	private:
		Vk_Device* _device;
		std::vector<T_StructureType> _cpuDataBuffer;
		size_t _count;
		size_t _maxCount;
		std::string _objName;
		std::mutex _localMutex;
		Vk_BufferSizeBehaviour _sizeBehaviour;
		Vk_BufferUpdateBehaviour _updateBehaviour;
		Vk_DataBufferLib::BufferType _type;
		std::string _associatedObject;

		int _bufferIndex;
		std::vector<VkBuffer> _buffer;
		std::vector<VkDeviceMemory> _bufferMemory;

		size_t getNextMaxCount(size_t oldMaxCount) {
			return Vk_DataBufferLib::getNextMaxCount(_sizeBehaviour, oldMaxCount);
		}

		/**
		* Resize a buffer to newMaxCount (amount of elements in the buffer). If
		* resize is a part of an update, use newFromOffset (in elements) to only
		* transfer the part of the data that we want to keep.
		*/
		void resizeDataBufferForUpdateStrategy(size_t newMaxCount, size_t useCount) {
			Vk_DataBufferLib::checkResizeAsserts(_objName, _count, newMaxCount, useCount);
			Vk_DataBufferLib::resizeMessage(_objName, _type, _sizeBehaviour, _maxCount, newMaxCount, _count);
			Vk_Logger::Warn(typeid(this), "Resizing buffer {0} from {1} to {2}", ("#Resize#" + _objName + _associatedObject), _maxCount, newMaxCount);

			_maxCount = newMaxCount;
			_count = useCount;

			std::uint64_t dataSize = static_cast<std::uint64_t>(vk_bufferSize());
			std::uint64_t maxSize = static_cast<std::uint64_t>(vk_maxBufferSize());

			for(int i=0; i<_buffer.size(); ++i){
				VkBuffer newBuffer;
				VkDeviceMemory newBufferMemory;
				try {
					VkBuffer buf = _buffer.at(i);
					VkDeviceMemory mem = _bufferMemory.at(i);
					Vk_DataBufferLib::createDeviceLocalBuffer(_device, _type, newBuffer, newBufferMemory, maxSize, Vk_DataBufferLib::Usage::Both);
					std::string nn = "#Resize#" + _objName + _associatedObject;
					Vk_DataBufferLib::copyGpuToGpu(_device, nn, buf, newBuffer, dataSize);
					Vk_DataBufferLib::destroyGpuBuffer(_device, buf, mem);
					buf = newBuffer;
					mem = newBufferMemory;
				}
				catch (const OutOfDeviceMemoryException&) {
					Vk_DataBufferLib::deviceLocalMemoryOverflowMessage(_device, _objName, maxSize);
					// copy to cpu first, remove old buffer and then copy back
					vkDestroyBuffer(_device->vk_lDev(), newBuffer, nullptr);
					getDataToCpu();
					Vk_DataBufferLib::destroyGpuBuffers(_device, _buffer, _bufferMemory);
					createDataBufferForUpdateStrategy(_cpuDataBuffer.data());
				}
			}
		}

		void updateDataBufferForUpdateStrategy(
			const T_StructureType* structuredData,
			size_t newCount,
			size_t newFrom,
			size_t newTo
		) {
			auto lock = std::lock_guard<std::mutex>(_localMutex);

			size_t dataCount = newCount - newFrom;
			std::uint64_t offsetSize = static_cast<std::uint64_t>(newFrom * sizeof(T_StructureType));
			std::uint64_t dataSize = static_cast<std::uint64_t>(dataCount * sizeof(T_StructureType));

			if(dataSize < sizeof(T_StructureType)){
				Vk_Logger::Error(typeid(this), "Update of object {0}-{1} failed: data size is {2} byte but must be greater than {3} byte!", _objName, _associatedObject, dataSize, sizeof(T_StructureType));
				return;
			}

			size_t newMaxCount;
			Vk_DataBufferLib::getNewBufferCount(_sizeBehaviour, _maxCount, newCount, newMaxCount);
			Vk_DataBufferLib::updateMessage(_objName, _type, _sizeBehaviour, _maxCount, newMaxCount, _count, newCount);

			if (newMaxCount > _maxCount) {
				resizeDataBufferForUpdateStrategy(newMaxCount, _count);
				_maxCount = newMaxCount;
			}

			copyDataToBufferForUpdateStrategy(structuredData, newCount, newFrom, newTo);

			_count = newCount;
		}

		void getDataToCpu() {
			auto lock = std::lock_guard<std::mutex>(_localMutex);
			std::uint64_t dataSize = vk_bufferSize();

			// create host buffer to copy all necessary data into cpu accessible memory, lets call it stagingBuffer
			VkBuffer stagingBuffer;
			VkDeviceMemory stagingBufferMemory;
			Vk_DataBufferLib::createStagingBuffer(_device, _type, stagingBuffer, stagingBufferMemory, dataSize, Vk_DataBufferLib::Usage::Destination);

			// copy data to staging buffer from non-cpu accessible vertex buffer
			// free memory afterwards
			std::string nn = "#Get" + _objName + _associatedObject;
			Vk_DataBufferLib::copyGpuToGpu(_device, nn, _buffer.at(_bufferIndex), stagingBuffer, dataSize);

			// map memory into variable to actually use it
			_cpuDataBuffer.clear();
			_cpuDataBuffer.resize(vk_bufferCount());
			Vk_DataBufferLib::copyGpuToCpu(_device, stagingBufferMemory, _cpuDataBuffer.data(), dataSize);

			Vk_DataBufferLib::destroyGpuBuffer(_device, stagingBuffer, stagingBufferMemory);
		}

		void createDataBufferForUpdateStrategy(const T_StructureType* structuredData) {
			auto lock = std::lock_guard<std::mutex>(_localMutex);

			std::uint64_t dataSize = static_cast<std::uint64_t>(vk_bufferSize());
			std::uint64_t maxSize = static_cast<std::uint64_t>(vk_maxBufferSize());

			if(dataSize < sizeof(T_StructureType)){
				Vk_Logger::Error(typeid(this), "Create of object {0}-{1} failed: data size is {2} byte but must be at least {3} byte!", _objName, _associatedObject, dataSize, sizeof(T_StructureType));
				return;
			}

			allocateBufferForUpdateStrategy(maxSize);

			// copy data from staging buffer to non-cpu accessible vertex buffer
			// free memory afterwards
			if (structuredData != nullptr) {
				copyDataToBufferForUpdateStrategy(structuredData, vk_bufferCount(),  0, vk_bufferCount());
			}
		}

		void allocateBufferForUpdateStrategy(std::uint64_t maxSize) {
			if(_updateBehaviour == Vk_BufferUpdateBehaviour::GlobalLock){
				// create one real buffer in non-cpu accessible memory
				VkBuffer lBuffer;
				VkDeviceMemory lBufferMemory;
				Vk_DataBufferLib::createDeviceLocalBuffer(_device, _type, lBuffer, lBufferMemory, maxSize, Vk_DataBufferLib::Usage::Both);
				_buffer.push_back(lBuffer);
				_bufferMemory.push_back(lBufferMemory);
			}
			else if(_updateBehaviour == Vk_BufferUpdateBehaviour::LazyDoubleBuffering){
				VkBuffer lBuffer;
				VkDeviceMemory lBufferMemory;
				Vk_DataBufferLib::createDeviceLocalBuffer(_device, _type, lBuffer, lBufferMemory, maxSize, Vk_DataBufferLib::Usage::Both);
				_buffer.push_back(lBuffer);
				_bufferMemory.push_back(lBufferMemory);
				_buffer.push_back(nullptr);
				_bufferMemory.push_back(nullptr);
			}
			else if(_updateBehaviour == Vk_BufferUpdateBehaviour::DoubleBuffering){
				// create two real _buffers in non-cpu accessible memory
				for(int i=0; i<2; ++i){
					VkBuffer lBuffer;
					VkDeviceMemory lBufferMemory;
					Vk_DataBufferLib::createDeviceLocalBuffer(_device, _type, lBuffer, lBufferMemory, maxSize, Vk_DataBufferLib::Usage::Both);
					_buffer.push_back(lBuffer);
					_bufferMemory.push_back(lBufferMemory);
				}
			}
			else if(_updateBehaviour == Vk_BufferUpdateBehaviour::Pinned_GlobalLock){
				// create one real _buffer in cpu-accessible memory
				VkBuffer lBuffer;
				VkDeviceMemory lBufferMemory;
				Vk_DataBufferLib::createDeviceLocalCPUAccessibleBuffer(_device, _type, lBuffer, lBufferMemory, maxSize, Vk_DataBufferLib::Usage::Both);
				_buffer.push_back(lBuffer);
				_bufferMemory.push_back(lBufferMemory);
			}
			else if(_updateBehaviour == Vk_BufferUpdateBehaviour::Pinned_DoubleBuffering){
				for(int i=0; i<2; ++i){
					VkBuffer lBuffer;
					VkDeviceMemory lBufferMemory;
					Vk_DataBufferLib::createDeviceLocalBuffer(_device, _type, lBuffer, lBufferMemory, maxSize, Vk_DataBufferLib::Usage::Both);
					_buffer.push_back(lBuffer);
					_bufferMemory.push_back(lBufferMemory);
				}
			}
			else {
				Vk_Logger::RuntimeError(typeid(this), "Unsupported update behaviour: {0}", Vk_BufferUpdateBehaviourToString(_updateBehaviour));
			}
		}

		void copyDataToBufferForUpdateStrategy(const T_StructureType* structuredData, size_t count, size_t from, size_t to){
			if(_updateBehaviour == Vk_BufferUpdateBehaviour::GlobalLock){
				// lock everything => potentially async, but in reality not really
				auto lock = AcquireGlobalLock("Vk_DataBuffer[copyDataToBufferForUpdateStrategy]");
				Vk_DataBufferLib::copyDataToBufferWithStaging(_device, _type, _buffer.at(0), structuredData, count, from, to, _objName, _associatedObject);
			}
			else if(_updateBehaviour == Vk_BufferUpdateBehaviour::DoubleBuffering){
				// copy to not used buffer and then flip them inside a synchronized bridge update
				// the update function is globally synched, so no need for mutexes here apart from the local one
				Vk_DataBufferLib::copyDataToBufferWithStaging(_device, _type, _buffer.at((_bufferIndex+1)%2), structuredData, count, from, to, _objName, _associatedObject);
				_device->bridge.addUpdateForNextFrame(
					[this](){ this->_bufferIndex = (this->_bufferIndex+1)%2; }
				);
			}
			else if(_updateBehaviour == Vk_BufferUpdateBehaviour::LazyDoubleBuffering){
				// first allocate new buffer
				// then copy data into it
				// then schedule index switch and deletion of old data
			}
			else if(_updateBehaviour == Vk_BufferUpdateBehaviour::Pinned_DoubleBuffering){
				
			}
			else if(_updateBehaviour == Vk_BufferUpdateBehaviour::Pinned_GlobalLock){
				
			}
			else {
				Vk_Logger::RuntimeError(typeid(this), "Unknown update behaviour strategy {0}", Vk_BufferUpdateBehaviourToString(_updateBehaviour));
			}
		}
	};
}