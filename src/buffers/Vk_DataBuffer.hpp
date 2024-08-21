#pragma once

#include <mutex>
#include <typeinfo>

#include "../Defines.h"
// #include "../Vk_Logger.hpp"
#include "../application/Vk_Device.hpp"
#include "../objects/Vk_Structures.hpp"

namespace VK4 {

	enum class BufferType {
		P,
		C,
		N,
		T,
		PC,
		PCN,
		PCNT,
		Index,
		Error
	};

	static std::string BufferTypeToString(BufferType type) {
		switch (type) {
		case BufferType::P: return "P";
		case BufferType::C: return "C";
		case BufferType::N: return "N";
		case BufferType::T: return "T";
		case BufferType::PC: return "PC";
		case BufferType::Index: return "Index";
		case BufferType::PCN: return "PCN";
		case BufferType::PCNT: return "PCNT";
		case BufferType::Error: return "Error";
		default: return "Unknown";
		}
	}

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
			_maxCount(getInitMaxCount(count, sizeBehaviour)),
			_objName(objName + "[" + std::string(typeid(T_StructureType).name()) + "]"),
			_sizeBehaviour(sizeBehaviour),
			_updateBehaviour(updateBehaviour),
			_type(getInitBufferType()),
			_associatedObject("(=" + associatedObject + "=)"),
			_buffer(nullptr),
			_bufferMemory(nullptr)
		{
			Vk_Logger::Log(typeid(this), GlobalCasters::castConstructorTitle(
				formatWithObjName(_objName, (std::string("Create Vertex Buffer ") + std::string(typeid(T_StructureType).name()) + _associatedObject))));

			checkAsserts();

			createDataBuffer(structuredData);
		}

		~Vk_DataBuffer() {
			Vk_Logger::Log(typeid(this), GlobalCasters::castDestructorTitle(
				formatWithObjName(_objName, (std::string("Destroy Vertex Buffer ") + std::string(typeid(T_StructureType).name()) + _associatedObject))));

			destroyGpuBuffer(_buffer, _bufferMemory);
		}

		/*
		* Get the Vulkan buffer descriptor
		*/
		VkBuffer vk_buffer() {
			return _buffer;
		}

		/*
		* Get the buffer binding point for the shaders
		*/
		//std::uint32_t vk_bindingPoint() {
		//	std::uint32_t binding = 0;
		//	//std::string name = std::string(typeid(T_StructureType).name());

		//	//if (name.compare(std::string(typeid(Geometry::Vk_Structure_Vertex).name())) == 0) {
		//	//	 binding = Geometry::Vk_Structure_Vertex::bindingPoint();
		//	//}
		//	//else if (name.compare(std::string(typeid(Geometry::Vk_Structure_Color).name())) == 0) {
		//	//	binding = Geometry::Vk_Structure_Color::bindingPoint();
		//	//}
		//	//else if (name.compare(std::string(typeid(Geometry::Vk_Structure_Normal).name())) == 0) {
		//	//	binding = Geometry::Vk_Structure_Normal::bindingPoint();
		//	//}

		//	return binding;
		//}

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
			resizeDataBuffer(newCount, _count);
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
				+ BufferTypeToString(_type)
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
			return getDeviceLocalBufferMaxMemory();
		}

		/*
		* Return the total available GPU/CPU shared memory. In general this is GPU main memory that
		* can be used by the GPU and usually corresponds to half of the computer main memory. This accesses
		* the device and returns the value the device returns.
		*/
		std::uint64_t vk_stagingBufferMemoryBudget() {
			return getStagingBufferMaxMemory();
		}

		/*
		* Return the element count the buffer will be initialized with given the parameter count
		* according to the buffer's initialization strategy.
		*/
		size_t vk_getRequiredInitCount(size_t count) {
			return getInitMaxCount(count, _sizeBehaviour);
		}


		/*
		* Get the new max buffer count given a hypothetical newCount based on the 
		* buffer's resize strategy.
		*/
		size_t vk_getNewRequiredMaxCount(size_t newCount) {
			size_t newMaxCount;
			getNewBufferCount(newCount, newMaxCount);
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
			updateDataBuffer(structuredData, newCount, newFrom, newTo);
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
		BufferType _type;
		std::string _associatedObject;

		VkBuffer _buffer;
		VkDeviceMemory _bufferMemory;

		enum class Usage {
			Both,
			Source,
			Destination,
		};

		BufferType getInitBufferType() {
			std::string name = std::string(typeid(T_StructureType).name());

			if (name.compare(std::string(typeid(Vk_Vertex_P).name())) == 0) {
				return BufferType::P;
			}
			if (name.compare(std::string(typeid(Vk_Vertex_C).name())) == 0) {
				return BufferType::C;
			}
			if (name.compare(std::string(typeid(Vk_Vertex_N).name())) == 0) {
				return BufferType::N;
			}
			if (name.compare(std::string(typeid(Vk_Vertex_T).name())) == 0) {
				return BufferType::T;
			}
			if (name.compare(std::string(typeid(Vk_Vertex_PC).name())) == 0) {
				return BufferType::PC;
			}
			else if (name.compare(std::string(typeid(Vk_Vertex_PCN).name())) == 0) {
				return BufferType::PCN;
			}
			else if (name.compare(std::string(typeid(Vk_Vertex_PCNT).name())) == 0) {
				return BufferType::PCNT;
			}
			else if (name.compare(std::string(typeid(VK4::index_type).name())) == 0) {
				return BufferType::Index;
			}

			Vk_Logger::RuntimeError(typeid(this), 
				std::string("Unable to set buffer type to [")
				+ GlobalCasters::castHighlightRed(name)
				+ std::string("]. Type is not supported")
			);

			return BufferType::Error;
		}

		size_t getInitMaxCount(size_t count, Vk_BufferSizeBehaviour sizeBehaviour) {
			if ((sizeBehaviour == Vk_BufferSizeBehaviour::Init_Empty_Grow_1_5) || (sizeBehaviour == Vk_BufferSizeBehaviour::Init_Empty_Grow_2)) {
				return static_cast<size_t>(std::ceil(1.0 * count));
			}

			if ((sizeBehaviour == Vk_BufferSizeBehaviour::Init_1_0_Grow_1_5) || (sizeBehaviour == Vk_BufferSizeBehaviour::Init_1_0_Grow_2)) {
				return static_cast<size_t>(std::ceil(1.0 * count));
			}

			if ((sizeBehaviour == Vk_BufferSizeBehaviour::Init_1_5_Grow_1_5) || (sizeBehaviour == Vk_BufferSizeBehaviour::Init_1_5_Grow_2)) {
				return static_cast<size_t>(std::ceil(1.5 * count));
			}

			return 0;
		}

		VkBufferUsageFlags getUsageFlags(Usage usage, bool staging=false){
			VkBufferUsageFlags usageFlags = 0;
			if (!staging) {
				usageFlags = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
				if (_type == BufferType::Index) {
					// we have an index buffer
					usageFlags = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
				}
			}
			if (usage == Usage::Both) usageFlags |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
			else if (usage == Usage::Destination) usageFlags |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
			else if (usage == Usage::Source) usageFlags |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

			return usageFlags;
		}

		size_t getNextMaxCount(size_t oldMaxCount) {
			if (
				(_sizeBehaviour == Vk_BufferSizeBehaviour::Init_1_0_Grow_2) ||
				(_sizeBehaviour == Vk_BufferSizeBehaviour::Init_1_5_Grow_2) ||
				(_sizeBehaviour == Vk_BufferSizeBehaviour::Init_Empty_Grow_2)) 
			{
				return static_cast<size_t>(std::ceil(oldMaxCount * 2.0));
			}

			if (
				(_sizeBehaviour == Vk_BufferSizeBehaviour::Init_1_0_Grow_1_5) || 
				(_sizeBehaviour == Vk_BufferSizeBehaviour::Init_1_5_Grow_1_5) ||
				(_sizeBehaviour == Vk_BufferSizeBehaviour::Init_Empty_Grow_1_5)) 
			{
				return static_cast<size_t>(std::ceil(oldMaxCount * 1.5));
			}

			Vk_Logger::RuntimeError(typeid(this), 
				std::string("Unable to get new max count from [")
				+ GlobalCasters::castHighlightRed(std::to_string(oldMaxCount))
				+ std::string("]. Behaviour characteristics is not supported")
			);

			return 0;
		}

		/**
		* Resize a buffer to newMaxCount (amount of elements in the buffer). If
		* resize is a part of an update, use newFromOffset (in elements) to only
		* transfer the part of the data that we want to keep.
		*/
		void resizeDataBuffer(size_t newMaxCount, size_t useCount) {
			checkResizeAsserts(newMaxCount, useCount);

			resizeMessage(newMaxCount);

			Vk_Logger::Warn(typeid(this), "Resizing buffer {0} from {1} to {2}", ("#Resize#" + _objName + _associatedObject), _maxCount, newMaxCount);

			_maxCount = newMaxCount;
			_count = useCount;

			std::uint64_t dataSize = static_cast<std::uint64_t>(vk_bufferSize());
			std::uint64_t maxSize = static_cast<std::uint64_t>(vk_maxBufferSize());

			VkBuffer newBuffer;
			VkDeviceMemory newBufferMemory;
			try {
				createDeviceLocalBuffer(newBuffer, newBufferMemory, maxSize, Usage::Both);
				copyGpuToGpu("#Resize#" + _objName + _associatedObject, _buffer, newBuffer, dataSize);
				destroyGpuBuffer(_buffer, _bufferMemory);
				_buffer = newBuffer;
				_bufferMemory = newBufferMemory;
			}
			catch (const OutOfDeviceMemoryException&) {
				deviceLocalMemoryOverflowMessage(maxSize);
				// copy to cpu first, remove old buffer and then copy back
				vkDestroyBuffer(_device->vk_lDev(), newBuffer, nullptr);
				getDataToCpu();
				destroyGpuBuffer(_buffer, _bufferMemory);
				createDataBuffer(_cpuDataBuffer.data());
			}
		}

		bool getNewBufferCount(const size_t newCount, size_t& newMaxCount) {
			bool resize = false;
			newMaxCount = _maxCount;
			while (newMaxCount < newCount) {
				newMaxCount = getNextMaxCount(newMaxCount);
				resize = true;
			}
			return resize;
		}

		void updateDataBuffer(
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
			getNewBufferCount(newCount, newMaxCount);

			updateMessage(_maxCount, newMaxCount, _count, newCount);

			if (newMaxCount > _maxCount) {
				resizeDataBuffer(newMaxCount, _count);
				_maxCount = newMaxCount;
			}

			VkBuffer stagingBuffer = VK_NULL_HANDLE;
			VkDeviceMemory stagingBufferMemory = VK_NULL_HANDLE;
			createStagingBuffer(stagingBuffer, stagingBufferMemory, dataSize, Usage::Source);

			copyCpuToGpu(structuredData + newFrom, stagingBufferMemory, dataSize);

			copyGpuToGpu("#Update#" + _objName + _associatedObject, stagingBuffer, _buffer, dataSize, 0, offsetSize);

			destroyGpuBuffer(stagingBuffer, stagingBufferMemory);

			_count = newCount;
		}

		void getDataToCpu() {
			auto lock = std::lock_guard<std::mutex>(_localMutex);
			std::uint64_t dataSize = vk_bufferSize();

			// create host buffer to copy all necessary data into cpu accessible memory, lets call it stagingBuffer
			VkBuffer stagingBuffer;
			VkDeviceMemory stagingBufferMemory;
			createStagingBuffer(stagingBuffer, stagingBufferMemory, dataSize, Usage::Destination);

			// copy data to staging buffer from non-cpu accessible vertex buffer
			// free memory afterwards
			copyGpuToGpu("#Get" + _objName + _associatedObject, _buffer, stagingBuffer, dataSize);

			// map memory into variable to actually use it
			_cpuDataBuffer.clear();
			_cpuDataBuffer.resize(vk_bufferCount());
			copyGpuToCpu(stagingBufferMemory, _cpuDataBuffer.data(), dataSize);

			destroyGpuBuffer(stagingBuffer, stagingBufferMemory);
		}

		void createDataBuffer(const T_StructureType* structuredData) {
			auto lock = std::lock_guard<std::mutex>(_localMutex);

			createMessage();

			std::uint64_t dataSize = static_cast<std::uint64_t>(vk_bufferSize());
			std::uint64_t maxSize = static_cast<std::uint64_t>(vk_maxBufferSize());

			if(dataSize < sizeof(T_StructureType)){
				Vk_Logger::Error(typeid(this), "Create of object {0}-{1} failed: data size is {2} byte but must be at least {3} byte!", _objName, _associatedObject, dataSize, sizeof(T_StructureType));
				return;
			}

			// create real buffer in non-cpu accessible memory
			createDeviceLocalBuffer(_buffer, _bufferMemory, maxSize, Usage::Both);

			// copy data from staging buffer to non-cpu accessible vertex buffer
			// free memory afterwards
			if (structuredData != nullptr) {
				// create host buffer to copy all necessary data into cpu accessible memory, lets call it stagingBuffer
				VkBuffer stagingBuffer = VK_NULL_HANDLE;
				VkDeviceMemory stagingBufferMemory = VK_NULL_HANDLE;

				// if the buffer is initially empty, no need to copy random stuff, just create the vertex buffer
				createStagingBuffer(stagingBuffer, stagingBufferMemory, dataSize, Usage::Source);

				// map memory into variable to actually use it
				copyCpuToGpu(structuredData, stagingBufferMemory, dataSize);

				copyGpuToGpu("#Create#" + _objName + _associatedObject, stagingBuffer, _buffer, dataSize);

				destroyGpuBuffer(stagingBuffer, stagingBufferMemory);
			}
		}

		void destroyGpuBuffer(VkBuffer buffer, VkDeviceMemory bufferMemory) {
			if(buffer != VK_NULL_HANDLE)
				vkDestroyBuffer(_device->vk_lDev(), buffer, nullptr);
			if(bufferMemory != VK_NULL_HANDLE)
				vkFreeMemory(_device->vk_lDev(), bufferMemory, nullptr);
		}

		void copyGpuToCpu(VkDeviceMemory gpuMemoryPtr, T_StructureType* cpuMemoryPtr, std::uint64_t size) {
			VkDevice lDev = _device->vk_lDev();

			void* data;
			vkMapMemory(lDev, gpuMemoryPtr, 0, static_cast<VkDeviceSize>(size), 0, &data);
			memcpy(static_cast<void*>(cpuMemoryPtr), data, size);
			vkUnmapMemory(lDev, gpuMemoryPtr);
		}

		void copyGpuToGpu(const std::string& objName, VkBuffer srcBuffer, VkBuffer dstBuffer, std::uint64_t size, std::uint64_t srcOffset=0, std::uint64_t dstOffset=0) {
			assert(size > srcOffset);
			_device->vk_copyBuffer(
				Vk_Device::CommandCapabilities::RuntimeCopy,
				srcBuffer, dstBuffer,
				static_cast<VkDeviceSize>(size),
				static_cast<VkDeviceSize>(srcOffset),
				static_cast<VkDeviceSize>(dstOffset),
				objName
			);
		}

		void copyCpuToGpu(const T_StructureType* cpuMemoryPtr, VkDeviceMemory gpuMemoryPtr, std::uint64_t size, std::uint64_t srcOffset=0, std::uint64_t dstOffset=0) {
			assert(size > srcOffset);
			VkDevice lDev = _device->vk_lDev();

			void* data;
			const T_StructureType* offsetCpuMemoryPtr = cpuMemoryPtr + (srcOffset /sizeof(T_StructureType));
			vkMapMemory(lDev, gpuMemoryPtr, static_cast<VkDeviceSize>(dstOffset), static_cast<VkDeviceSize>(size), 0, &data);
			memcpy(data, static_cast<const void*>(offsetCpuMemoryPtr), static_cast<size_t>(size));
			vkUnmapMemory(lDev, gpuMemoryPtr);
		}

		void createDeviceLocalBuffer(VkBuffer& buffer, VkDeviceMemory& memory, std::uint64_t size, Usage usage) {
			VkBufferUsageFlags usageFlags = getUsageFlags(usage);
			_device->vk_createBuffer(
				usageFlags,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				buffer,
				memory,
				static_cast<VkDeviceSize>(size)
			);
		}

		std::uint64_t getDeviceLocalBufferMaxMemory() {
			return _device->vk_queryInstalledMemory(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		}

		void createStagingBuffer(VkBuffer& buffer, VkDeviceMemory& memory, std::uint64_t size, Usage usage) {
			// it may be that memcpy does not copy the data right away (chaching and so on) ensure either
			//  VK_MEMORY_PROPERTY_HOST_COHERENT_BIT is set for memory area (slightly worse performance)
			//  or vkFlushMappedMemoryRanges after writing and vkInvalidateMappedMemoryRanges after reading
			VkBufferUsageFlags usageFlags = getUsageFlags(usage, true);

			_device->vk_createBuffer(
				usageFlags,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				buffer,
				memory,
				static_cast<VkDeviceSize>(size)
			);
		}

		std::uint64_t getStagingBufferMaxMemory() {
			return _device->vk_queryInstalledMemory(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		}

		std::uint64_t getStagingBufferMemoryBudget() {
			return _device->vk_queryMemoryBudget(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		}

		std::uint64_t bestPowerOfTwo(std::uint64_t size) {
			std::uint64_t temp = size;
			int counter = 0;
			while (temp > 0) {
				temp = temp >> 1;
				counter++;
			}
			return static_cast<std::uint64_t>(std::pow(2, counter));
		}

		inline void checkResizeAsserts(size_t newCount, size_t newFrom) {
			ASSERT_MSG(newCount >= 0,
				formatWithObjName(_objName,
					std::string("New count must be >= 0 but is ")
					+ GlobalCasters::castHighlightRed(std::to_string(newCount))
				).c_str());

			ASSERT_MSG(newCount >= _count,
				formatWithObjName(_objName,
					std::string("New count must be >= current data count. Current data count is ")
					+ GlobalCasters::castHighlightYellow(std::to_string(_count))
					+ std::string(" but new count is ")
					+ GlobalCasters::castHighlightRed(std::to_string(newCount))
				).c_str());

			ASSERT_MSG(newCount >= newFrom,
				formatWithObjName(_objName,
					std::string("New count must be > newFrom. Current new count is ")
					+ GlobalCasters::castHighlightYellow(std::to_string(newCount))
					+ std::string(" but newFrom is ")
					+ GlobalCasters::castHighlightRed(std::to_string(newFrom))
				).c_str());
		}

		inline void checkAsserts() {
			ASSERT_MSG(vk_bufferCount() >= 0,
				formatWithObjName(_objName,
					std::string("Buffer size >= 0 required but is ")
					+ GlobalCasters::castHighlightRed(std::to_string(vk_bufferCount()))
				).c_str());
		}

		//inline void updateMessage(int newFrom, int newCount) {
		//	Vk_Logger::Trace(typeid(this), 
		//		formatWithObjName(_objName, (
		//			GlobalCasters::castYellow("\n\tBuffer update: ") + std::string("Update Vertex Buffer\n")
		//			+ GlobalCasters::castYellow("\tBuffer update: ") + std::string("           new from: ") + std::to_string(newFrom) + "\n"
		//			+ GlobalCasters::castYellow("\tBuffer update: ") + std::string("   new content size: ") + std::to_string(newCount) + "\n"
		//			))
		//	);
		//}

		inline void createMessage() {
			Vk_Logger::Trace(typeid(this), 
				formatWithObjName(_objName, (
					GlobalCasters::castYellow("\n\tBuffer create: ") + std::string("Create new buffer\n")
					+ GlobalCasters::castYellow("\tBuffer create: ") + std::string("            type: ") + BufferTypeToString(_type) + "\n"
					+ GlobalCasters::castYellow("\tBuffer create: ") + std::string(" characteristics: ") + Vk_BufferSizeBehaviourToString(_sizeBehaviour) + "\n"
					+ GlobalCasters::castYellow("\tBuffer create: ") + std::string("       max count: ") + std::to_string(_maxCount) + "\n"
					+ GlobalCasters::castYellow("\tBuffer create: ") + std::string("  required count: ") + std::to_string(_count) + "\n"
				))
			);
		}

		inline void updateMessage(size_t oldMaxCount, size_t newMaxCount, size_t oldDataCount, size_t newDataCount) {
			Vk_Logger::Trace(typeid(this), 
				formatWithObjName(_objName, (
					GlobalCasters::castYellow("\n\tBuffer update: ") + std::string("Update buffer\n")
					+ GlobalCasters::castYellow("\tBuffer update: ") + std::string("            type: ") + BufferTypeToString(_type) + "\n"
					+ GlobalCasters::castYellow("\tBuffer update: ") + std::string(" characteristics: ") + Vk_BufferSizeBehaviourToString(_sizeBehaviour) + "\n"
					+ GlobalCasters::castYellow("\tBuffer update: ") + std::string("       old count: ") + std::to_string(oldDataCount) + "\n"
					+ GlobalCasters::castYellow("\tBuffer update: ") + std::string("       new count: ") + std::to_string(newDataCount) + "\n"
					+ GlobalCasters::castYellow("\tBuffer update: ") + std::string("   old max count: ") + std::to_string(oldMaxCount) + "\n"
					+ GlobalCasters::castYellow("\tBuffer update: ") + std::string("   new max count: ") + std::to_string(newMaxCount) + "\n"
					))
			);
		}

		inline void deviceLocalMemoryOverflowMessage(size_t requested) {
			Vk_Logger::Warn(typeid(this), 
				formatWithObjName(_objName, (
					GlobalCasters::castYellow("Device-local memory overflow:")
					+ std::string(" available memory budget is ")
					+ GlobalCasters::castHighlightYellow(std::to_string(vk_localBufferMemoryBudget()))
					+ std::string(" but requested memory for device-local resize is ")
					+ GlobalCasters::castHighlightYellow(std::to_string(vk_localBufferMemoryBudget() + requested))
				))
			);
		}

		inline void resizeMessage(size_t newMaxCount) {
			Vk_Logger::Trace(typeid(this), 
				formatWithObjName(_objName, (
					GlobalCasters::castYellow("\n\tBuffer resize: ") + std::string("Resize buffer\n")
					+ GlobalCasters::castYellow("\tBuffer resize: ") + std::string("            type: ") + BufferTypeToString(_type) + "\n"
					+ GlobalCasters::castYellow("\tBuffer resize: ") + std::string(" characteristics: ") + Vk_BufferSizeBehaviourToString(_sizeBehaviour) + "\n"
					+ GlobalCasters::castYellow("\tBuffer resize: ") + std::string("   old max count: ") + std::to_string(_maxCount) + "\n"
					+ GlobalCasters::castYellow("\tBuffer resize: ") + std::string("   new max count: ") + std::to_string(newMaxCount) + "\n"
					+ GlobalCasters::castYellow("\tBuffer resize: ") + std::string("  required count: ") + std::to_string(_count) + "\n"
				))
			);
		}

		inline void warnOutOfSize() {
			Vk_Logger::Warn(typeid(this), 
				formatWithObjName(_objName, (
					GlobalCasters::castYellow("\n\tBuffer out of size: ") + std::string("\n******************************************************************************************\n")
					+ GlobalCasters::castYellow("\tBuffer out of size: ") + std::string("Extendable Vertex Buffer of type \"")
					+ GlobalCasters::castYellow("\tBuffer out of size: ") + std::string(typeid(T_StructureType).name())
					+ GlobalCasters::castYellow("\tBuffer out of size: ") + std::string("\" has initial size ")
					+ GlobalCasters::castYellow("\tBuffer out of size: ") + std::to_string(_maxCount)
					+ GlobalCasters::castYellow("\tBuffer out of size: ") + std::string(" but requires initial size of ")
					+ GlobalCasters::castYellow("\tBuffer out of size: ") + std::to_string(_count)
					+ GlobalCasters::castYellow("\tBuffer out of size: ") + std::string("\nThe max size is increased to match. Consider allocating more initial space.")
					+ GlobalCasters::castYellow("\tBuffer out of size: ") + std::string("\n******************************************************************************************\n")
				))
			);
		}

		inline std::string formatWithObjName(std::string name, std::string message) {
			return std::string("[") + name + std::string("] ") + message;
		}
	};
}