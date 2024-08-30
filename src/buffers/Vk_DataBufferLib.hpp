#pragma once

#include "../Defines.h"
#include "../application/Vk_Device.hpp"
#include "../objects/Vk_Structures.hpp"
#include "../Vk_Lib.hpp"

namespace VK4 {
    class Vk_DataBufferLib {
    public:
        enum class Usage {
			Both,
			Source,
			Destination,
		};

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

		template<class T_StructureType>
		struct StructuredData {
			size_t count;
			const T_StructureType* data;
		};

        template<class T_StructureType>
        static BufferType getInitBufferType() {
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

			Vk_Logger::RuntimeError(typeid(NoneObj),
				std::string("Unable to set buffer type to [")
				+ GlobalCasters::castHighlightRed(name)
				+ std::string("]. Type is not supported")
			);

			return BufferType::Error;
		}

        static size_t getInitMaxCount(size_t count, Vk_BufferSizeBehaviour sizeBehaviour) {
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

        static size_t getNextMaxCount(
            Vk_BufferSizeBehaviour sizeBehaviour, 
            size_t oldMaxCount
        ) {
			if (
				(sizeBehaviour == Vk_BufferSizeBehaviour::Init_1_0_Grow_2) ||
				(sizeBehaviour == Vk_BufferSizeBehaviour::Init_1_5_Grow_2) ||
				(sizeBehaviour == Vk_BufferSizeBehaviour::Init_Empty_Grow_2)) 
			{
				return static_cast<size_t>(std::ceil(oldMaxCount * 2.0));
			}

			if (
				(sizeBehaviour == Vk_BufferSizeBehaviour::Init_1_0_Grow_1_5) || 
				(sizeBehaviour == Vk_BufferSizeBehaviour::Init_1_5_Grow_1_5) ||
				(sizeBehaviour == Vk_BufferSizeBehaviour::Init_Empty_Grow_1_5)) 
			{
				return static_cast<size_t>(std::ceil(oldMaxCount * 1.5));
			}

			Vk_Logger::RuntimeError(typeid(NoneObj), 
				std::string("Unable to get new max count from [")
				+ GlobalCasters::castHighlightRed(std::to_string(oldMaxCount))
				+ std::string("]. Behaviour characteristics is not supported")
			);

			return 0;
		}

        static bool getNewBufferCount(
            Vk_BufferSizeBehaviour sizeBehaviour,
            const size_t oldMaxCount,
            const size_t newCount, 
            size_t& newMaxCount
        ) {
			bool resize = false;
			newMaxCount = oldMaxCount;
			while (newMaxCount < newCount) {
				newMaxCount = getNextMaxCount(sizeBehaviour, newMaxCount);
				resize = true;
			}
			return resize;
		}

        static VkBufferUsageFlags getUsageFlags(BufferType type, Usage usage, bool staging=false){
			VkBufferUsageFlags usageFlags = 0;
			if (!staging) {
				usageFlags = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
				if (type == BufferType::Index) {
					// we have an index buffer
					usageFlags = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
				}
			}
			if (usage == Usage::Both) usageFlags |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
			else if (usage == Usage::Destination) usageFlags |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
			else if (usage == Usage::Source) usageFlags |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

			return usageFlags;
		}

        static void checkResizeAsserts(
            const std::string& objName, 
            size_t count,
            size_t newCount, 
            size_t newFrom
        ) {
			ASSERT_MSG(newCount >= 0,
				Vk_Lib::formatWithObjName(objName,
					std::string("New count must be >= 0 but is ")
					+ GlobalCasters::castHighlightRed(std::to_string(newCount))
				).c_str());

			ASSERT_MSG(newCount >= count,
				Vk_Lib::formatWithObjName(objName,
					std::string("New count must be >= current data count. Current data count is ")
					+ GlobalCasters::castHighlightYellow(std::to_string(count))
					+ std::string(" but new count is ")
					+ GlobalCasters::castHighlightRed(std::to_string(newCount))
				).c_str());

			ASSERT_MSG(newCount >= newFrom,
				Vk_Lib::formatWithObjName(objName,
					std::string("New count must be > newFrom. Current new count is ")
					+ GlobalCasters::castHighlightYellow(std::to_string(newCount))
					+ std::string(" but newFrom is ")
					+ GlobalCasters::castHighlightRed(std::to_string(newFrom))
				).c_str());
		}

        static void checkAsserts(
            const std::string& objName,
            size_t bufferCount
        ) {
			ASSERT_MSG(bufferCount >= 0,
				Vk_Lib::formatWithObjName(objName,
					std::string("Buffer size >= 0 required but is ")
					+ GlobalCasters::castHighlightRed(std::to_string(bufferCount))
				).c_str());
		}

		static void createMessage(
            const std::string& objName,
            BufferType type,
            Vk_BufferSizeBehaviour sizeBehaviour,
            uint64_t maxCount,
            uint64_t count
        ) {
			Vk_Logger::Trace(typeid(NoneObj), 
				Vk_Lib::formatWithObjName(objName, (
					GlobalCasters::castYellow("\n\tBuffer create: ") + std::string("Create new buffer\n")
					+ GlobalCasters::castYellow("\tBuffer create: ") + std::string("            type: ") + BufferTypeToString(type) + "\n"
					+ GlobalCasters::castYellow("\tBuffer create: ") + std::string(" characteristics: ") + Vk_BufferSizeBehaviourToString(sizeBehaviour) + "\n"
					+ GlobalCasters::castYellow("\tBuffer create: ") + std::string("       max count: ") + std::to_string(maxCount) + "\n"
					+ GlobalCasters::castYellow("\tBuffer create: ") + std::string("  required count: ") + std::to_string(count) + "\n"
				))
			);
		}

		static void updateMessage(
            const std::string& objName, 
            BufferType type,
            Vk_BufferSizeBehaviour sizeBehaviour,
			Vk_BufferUpdateBehaviour updateBehaviour,
            size_t oldMaxCount, 
            size_t newMaxCount, 
            size_t oldDataCount, 
            size_t newDataCount
        ) {
			Vk_Logger::Trace(typeid(NoneObj), 
				Vk_Lib::formatWithObjName(objName, (
					GlobalCasters::castYellow("\n\tBuffer update: ") + std::string("Update buffer\n")
					+ GlobalCasters::castYellow("\tBuffer update: ") + std::string("            type: ") + BufferTypeToString(type) + "\n"
					+ GlobalCasters::castYellow("\tBuffer update: ") + std::string("  size behaviour: ") + Vk_BufferSizeBehaviourToString(sizeBehaviour) + "\n"
					+ GlobalCasters::castYellow("\tBuffer update: ") + std::string(" update behavour: ") + Vk_BufferUpdateBehaviourToString(updateBehaviour) + "\n"
					+ GlobalCasters::castYellow("\tBuffer update: ") + std::string("       old count: ") + std::to_string(oldDataCount) + "\n"
					+ GlobalCasters::castYellow("\tBuffer update: ") + std::string("       new count: ") + std::to_string(newDataCount) + "\n"
					+ GlobalCasters::castYellow("\tBuffer update: ") + std::string("   old max count: ") + std::to_string(oldMaxCount) + "\n"
					+ GlobalCasters::castYellow("\tBuffer update: ") + std::string("   new max count: ") + std::to_string(newMaxCount) + "\n"
				))
			);
		}

        static std::uint64_t getDeviceLocalBufferMaxMemory(
            Vk_Device* device
        ) {
			return device->vk_queryInstalledMemory(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		}

		static void deviceLocalMemoryOverflowMessage(
            Vk_Device* device,
            const std::string& objName, 
            size_t requested
        ) {
            auto devMem = getDeviceLocalBufferMaxMemory(device);
			Vk_Logger::Warn(typeid(NoneObj), 
				Vk_Lib::formatWithObjName(objName, (
					GlobalCasters::castYellow("Device-local memory overflow:")
					+ std::string(" available memory budget is ")
					+ GlobalCasters::castHighlightYellow(std::to_string(devMem))
					+ std::string(" but requested memory for device-local resize is ")
					+ GlobalCasters::castHighlightYellow(std::to_string(devMem + requested))
				))
			);
		}

		static void resizeMessage(
            const std::string& objName,
            BufferType type,
            Vk_BufferSizeBehaviour sizeBehaviour,
            size_t maxCount,
            size_t newMaxCount,
            size_t count
        ) {
			Vk_Logger::Trace(typeid(NoneObj), 
				Vk_Lib::formatWithObjName(objName, (
					GlobalCasters::castYellow("\n\tBuffer resize: ") + std::string("Resize buffer\n")
					+ GlobalCasters::castYellow("\tBuffer resize: ") + std::string("            type: ") + BufferTypeToString(type) + "\n"
					+ GlobalCasters::castYellow("\tBuffer resize: ") + std::string(" characteristics: ") + Vk_BufferSizeBehaviourToString(sizeBehaviour) + "\n"
					+ GlobalCasters::castYellow("\tBuffer resize: ") + std::string("   old max count: ") + std::to_string(maxCount) + "\n"
					+ GlobalCasters::castYellow("\tBuffer resize: ") + std::string("   new max count: ") + std::to_string(newMaxCount) + "\n"
					+ GlobalCasters::castYellow("\tBuffer resize: ") + std::string("  required count: ") + std::to_string(count) + "\n"
				))
			);
		}

        template<class T_StructureType>
		static void warnOutOfSize(
            const std::string& objName,
            size_t maxCount,
            size_t count
        ) {
			Vk_Logger::Warn(typeid(NoneObj), 
				Vk_Lib::formatWithObjName(objName, (
					GlobalCasters::castYellow("\n\tBuffer out of size: ") + std::string("\n******************************************************************************************\n")
					+ GlobalCasters::castYellow("\tBuffer out of size: ") + std::string("Extendable Vertex Buffer of type \"")
					+ GlobalCasters::castYellow("\tBuffer out of size: ") + std::string(typeid(T_StructureType).name())
					+ GlobalCasters::castYellow("\tBuffer out of size: ") + std::string("\" has initial size ")
					+ GlobalCasters::castYellow("\tBuffer out of size: ") + std::to_string(maxCount)
					+ GlobalCasters::castYellow("\tBuffer out of size: ") + std::string(" but requires initial size of ")
					+ GlobalCasters::castYellow("\tBuffer out of size: ") + std::to_string(count)
					+ GlobalCasters::castYellow("\tBuffer out of size: ") + std::string("\nThe max size is increased to match. Consider allocating more initial space.")
					+ GlobalCasters::castYellow("\tBuffer out of size: ") + std::string("\n******************************************************************************************\n")
				))
			);
		}

		/*
		 * Destroys the Vulkan objects and clears the vectors
		*/
        static void destroyGpuBuffers(
            Vk_Device* device,
            std::vector<VkBuffer>& buffers, 
            std::vector<VkDeviceMemory>& buffersMemory
        ){
			for(int i=0; i<buffers.size(); ++i){
				destroyGpuBuffer(device, buffers.at(i), buffersMemory.at(i));
			}
			buffers.clear();
			buffersMemory.clear();
		}

		/*
		 * Destroys the associated Vulkan objects. Does NOT set them to nullptr.
		*/
		static void destroyGpuBuffer(
            Vk_Device* device,
            VkBuffer buffer, 
            VkDeviceMemory bufferMemory
        ) {
			if(buffer != VK_NULL_HANDLE)
				vkDestroyBuffer(device->vk_lDev(), buffer, nullptr);
			if(bufferMemory != VK_NULL_HANDLE)
				vkFreeMemory(device->vk_lDev(), bufferMemory, nullptr);
		}

        template<class T_StructureType>
		static void copyGpuToCpu(
            Vk_Device* device,
            VkDeviceMemory gpuMemoryPtr, 
            T_StructureType* cpuMemoryPtr, 
            std::uint64_t copyByteSize
        ) {
			VkDevice lDev = device->vk_lDev();

			void* data;
			vkMapMemory(lDev, gpuMemoryPtr, 0, static_cast<VkDeviceSize>(copyByteSize), 0, &data);
			memcpy(static_cast<void*>(cpuMemoryPtr), data, copyByteSize);
			vkUnmapMemory(lDev, gpuMemoryPtr);
		}

		static void copyGpuToGpu(
            Vk_Device* device,
            const std::string& objName, 
            VkBuffer srcBuffer,
			std::uint64_t srcBufferSize, 
            VkBuffer dstBuffer, 
			std::uint64_t dstBufferSize,
            std::uint64_t copyByteSize, 
            std::uint64_t srcByteOffset=0, 
            std::uint64_t dstByteOffset=0
        ) {
			// make sure that we access inside the source buffer
			assert(srcBufferSize >= srcByteOffset + copyByteSize);
			// make sure that we access inside the dst buffer
			assert(dstBufferSize >= dstByteOffset + copyByteSize);

			device->vk_copyBuffer(
				Vk_Device::CommandCapabilities::RuntimeCopy,
				srcBuffer, dstBuffer,
				static_cast<VkDeviceSize>(copyByteSize),
				static_cast<VkDeviceSize>(srcByteOffset),
				static_cast<VkDeviceSize>(dstByteOffset),
				objName
			);
		}

        template<class T_StructureType>
		static void copyCpuToGpu(
            Vk_Device* device,
            const Vk_DataBufferLib::StructuredData<T_StructureType>& structuredData, 
            VkDeviceMemory gpuMemoryPtr, 
            std::uint64_t copyByteSize, 
            std::uint64_t srcByteOffset=0, 
            std::uint64_t dstByteOffset=0
        ) {
			// need srcByteOffset to be at most the total data size-1
			assert(structuredData.count*sizeof(T_StructureType) >= srcByteOffset + copyByteSize);

			VkDevice lDev = device->vk_lDev();

			void* data;
			const T_StructureType* offsetCpuMemoryPtr = structuredData.data + (srcByteOffset /sizeof(T_StructureType));
			vkMapMemory(lDev, gpuMemoryPtr, static_cast<VkDeviceSize>(dstByteOffset), static_cast<VkDeviceSize>(copyByteSize), 0, &data);
			memcpy(data, static_cast<const void*>(offsetCpuMemoryPtr), static_cast<size_t>(copyByteSize));
			vkUnmapMemory(lDev, gpuMemoryPtr);
		}

		static void createDeviceLocalBuffer(
            Vk_Device* device,
            BufferType type,
            VkBuffer& buffer, 
            VkDeviceMemory& memory, 
            std::uint64_t size, 
            Usage usage
        ) {
			VkBufferUsageFlags usageFlags = getUsageFlags(type, usage);
			device->vk_createBuffer(
				usageFlags,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				buffer,
				memory,
				static_cast<VkDeviceSize>(size)
			);
		}

		static void createDeviceLocalCPUAccessibleBuffer(
            Vk_Device* device,
            BufferType type,
            VkBuffer& buffer, 
            VkDeviceMemory& memory, 
            std::uint64_t size, 
            Usage usage
        ) {
			VkBufferUsageFlags usageFlags = getUsageFlags(type, usage);
			device->vk_createBuffer(
				usageFlags,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				buffer,
				memory,
				static_cast<VkDeviceSize>(size)
			);
		}

        static void createStagingBuffer(
            Vk_Device* device,
            BufferType type,
            VkBuffer& buffer, 
            VkDeviceMemory& memory, 
            std::uint64_t size, 
            Usage usage
        ) {
			// it may be that memcpy does not copy the data right away (chaching and so on) ensure either
			//  VK_MEMORY_PROPERTY_HOST_COHERENT_BIT is set for memory area (slightly worse performance)
			//  or vkFlushMappedMemoryRanges after writing and vkInvalidateMappedMemoryRanges after reading
			VkBufferUsageFlags usageFlags = getUsageFlags(type, usage, true);

			device->vk_createBuffer(
				usageFlags,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				buffer,
				memory,
				static_cast<VkDeviceSize>(size)
			);
		}

        template<class T_StructureType>
        static void copyDataToBufferWithStaging(
            Vk_Device* device,
            BufferType type,
            VkBuffer buffer, 
			uint64_t bufferByteSize,
            const Vk_DataBufferLib::StructuredData<T_StructureType>& structuredData,
            size_t from, 
            size_t to,
            const std::string& objName = "",
            const std::string& associatedObject = ""
        ){
            if(from > to){
                Vk_Logger::RuntimeError(typeid(NoneObj), "Data [from, to] must be an interval of positive length but is {0} items long!", to-from);
            }

			// create host buffer to copy all necessary data into cpu accessible memory, lets call it stagingBuffer
			VkBuffer stagingBuffer = VK_NULL_HANDLE;
			VkDeviceMemory stagingBufferMemory = VK_NULL_HANDLE;
			uint64_t dataSize = static_cast<uint64_t>(structuredData.count * sizeof(T_StructureType));
			uint64_t byteFrom = static_cast<uint64_t>(from * sizeof(T_StructureType));
			uint64_t byteTo = static_cast<uint64_t>(to * sizeof(T_StructureType));

			// if the buffer is initially empty, no need to copy random stuff, just create the vertex buffer
			// auto t1 = std::chrono::high_resolution_clock::now();
			createStagingBuffer(device, type, stagingBuffer, stagingBufferMemory, byteTo - byteFrom, Usage::Source);
			// auto t2 = std::chrono::high_resolution_clock::now();
			// std::cout << "############################333" << std::endl << std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count() << std::endl << "############################333" << std::endl;

            std::string nn = "#Create#" + objName + associatedObject;
			// map memory into variable to actually use it
			uint64_t copyByteSize = byteTo - byteFrom;
			// srcByteOffset = byteFrom, dstByteOffset = 0 because that is the staging buffer offset that only houses the new data
			copyCpuToGpu(device, structuredData, stagingBufferMemory, copyByteSize, byteFrom, 0);
			// srcByteOffset = 0 because that is the staging buffer offset that only houses the new data, 
			// dstByteOffset = byteFrom because we need to place the data in the right spot
			copyGpuToGpu(device, nn, stagingBuffer, byteTo - byteFrom, buffer, bufferByteSize, copyByteSize, 0, byteFrom);
			destroyGpuBuffer(device, stagingBuffer, stagingBufferMemory);
		}

		template<class T_StructureType>
        static void copyDataToBufferDirect(
            Vk_Device* device,
            BufferType type,
            VkDeviceMemory bufferMemory, 
			uint64_t bufferByteSize,
            const Vk_DataBufferLib::StructuredData<T_StructureType>& structuredData,
            size_t from, 
            size_t to,
            const std::string& objName = "",
            const std::string& associatedObject = ""
        ){
            if(from > to){
                Vk_Logger::RuntimeError(typeid(NoneObj), "Data [from, to] must be an interval of positive length but is {0} items long!", to-from);
            }
			uint64_t byteFrom = static_cast<uint64_t>(from * sizeof(T_StructureType));
			uint64_t byteTo = static_cast<uint64_t>(to * sizeof(T_StructureType));

            // std::string nn = "#Create#" + objName + associatedObject;
			// map memory into variable to actually use it
			uint64_t copyByteSize = byteTo - byteFrom;
			// srcByteOffset = byteFrom, dstByteOffset = 0 because that is the staging buffer offset that only houses the new data
			copyCpuToGpu(device, structuredData, bufferMemory, copyByteSize, byteFrom, byteFrom);
		}

        static std::uint64_t getStagingBufferMaxMemory(
            Vk_Device* device
        ) {
			return device->vk_queryInstalledMemory(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		}

		static std::uint64_t getStagingBufferMemoryBudget(
            Vk_Device* device
        ) {
			return device->vk_queryMemoryBudget(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		}
    };
}