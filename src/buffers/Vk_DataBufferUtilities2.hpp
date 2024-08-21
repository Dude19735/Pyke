#pragma once

#include <exception>

#include "../Defines.h"
#include "../Vk_Utils.hpp"
#include "../application/Vk_Device.hpp"

namespace VK4 {

	struct OutOfDeviceMemoryException : public std::exception
	{
		const char* what() const throw ()
		{
			return "GPU Device out of memory";
		}
	};

	class Vk_DataBufferUtilities {
	public:
		static void createBuffer(
			Vk_Device* const device,
			VkDeviceSize size,
			VkBufferUsageFlags usage,
			VkMemoryPropertyFlags properties,
			VkBuffer& buffer,
			VkDeviceMemory& bufferMemory) 
		{
			VkDevice lDev = device->vk_lDev();
			VkPhysicalDevice pDev = device->vk_pDev();

			VkBufferCreateInfo bufferInfo{};
			bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			bufferInfo.size = size;
			bufferInfo.usage = usage;
			bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

			VkResult res = vkCreateBuffer(lDev, &bufferInfo, nullptr, &buffer);
			if (res != VK_SUCCESS) {
				Vk_Logger::RuntimeError(typeid(this), "failed to create buffer!");
			}

			VkMemoryRequirements memRequirements;
			vkGetBufferMemoryRequirements(lDev, buffer, &memRequirements);

			VkMemoryAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			allocInfo.allocationSize = memRequirements.size;
			allocInfo.memoryTypeIndex =
				Vk_Utils::findMemoryType(pDev, memRequirements.memoryTypeBits, properties);

			res = vkAllocateMemory(lDev, &allocInfo, nullptr, &bufferMemory);
			if (res != VK_SUCCESS) {
				if (res == VK_ERROR_OUT_OF_DEVICE_MEMORY) {
					throw OutOfDeviceMemoryException();
				}
				else {
					Vk_Logger::RuntimeError(typeid(this), "failed to allocate buffer memory!");
				}
			}

			vkBindBufferMemory(lDev, buffer, bufferMemory, 0);
		}

		static VkCommandBuffer beginSingleTimeCommands(
			Vk_Device* const device,
			VkCommandPool commandPool
		) {
			// memory transfer operations are executed using command buffers
			// => allocate temporary command buffer
			VkCommandBufferAllocateInfo allocInfo = {};
			allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			allocInfo.commandPool = commandPool; // may want to specify another command pool for this kind of short lived command buffers
			allocInfo.commandBufferCount = 1;

			VkCommandBuffer commandBuffer;
			vkAllocateCommandBuffers(device->vk_lDev(), &allocInfo, &commandBuffer);

			// start recoding the command
			// VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT not necessary, because this is an execute and wait until finished kind
			// of methode, therefore use
			// VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT to tell the driver about our intentions
			VkCommandBufferBeginInfo beginInfo = {};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

			vkBeginCommandBuffer(commandBuffer, &beginInfo); // begin recording command

			return commandBuffer;
		}

		static void endSingleTimeCommands(
			Vk_Device* const device,
			VkCommandPool commandPool,
			VkCommandBuffer commandBuffer
		) {
			vkEndCommandBuffer(commandBuffer); // end recording command

			// submit the command to the graphics queue
			VkSubmitInfo submitInfo = {};
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &commandBuffer;

			Vk_ThreadSafe::Vk_ThreadSafe_QueueSubmit(&device->vk_graphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
			Vk_ThreadSafe::Vk_ThreadSafe_QueueWaitIdle(device->vk_graphicsQueue());

			// free command buffer memory
			vkFreeCommandBuffers(device->vk_lDev(), commandPool, 1, &commandBuffer);
		}

		static void copyBuffer(
			Vk_Device* const device,
			VkCommandPool commandPool,
			VkBuffer srcBuffer,
			VkBuffer dstBuffer,
			VkDeviceSize size,
			VkDeviceSize srcOffset = 0,
			VkDeviceSize dstOffset = 0
		) {

			VkCommandBuffer commandBuffer = Vk_DataBufferUtilities::beginSingleTimeCommands(device, commandPool);

			VkBufferCopy copyRegion = {};
			copyRegion.srcOffset = srcOffset; // optional
			copyRegion.dstOffset = dstOffset; // optional
			copyRegion.size = size;
			vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

			Vk_DataBufferUtilities::endSingleTimeCommands(device, commandPool, commandBuffer);
		}

		static void copyBuffer(
			Vk_Device* const device,
			VkCommandPool commandPool,
			VkBuffer srcBuffer,
			VkDeviceSize srcOffset,
			VkBuffer dstBuffer,
			VkDeviceSize dstOffset,
			VkDeviceSize size
		) {

			VkCommandBuffer commandBuffer = Vk_DataBufferUtilities::beginSingleTimeCommands(device, commandPool);

			VkBufferCopy copyRegion = {};
			copyRegion.srcOffset = srcOffset; // optional
			copyRegion.dstOffset = dstOffset; // optional
			copyRegion.size = size;
			vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

			Vk_DataBufferUtilities::endSingleTimeCommands(device, commandPool, commandBuffer);
		}
	};
}