// #pragma once

// #include <vulkan/vulkan.h>

// #include "../../../Defines.h"
// #include "../../../Vk_Logger.h"
// #include "../../application/Vk_Application.h"

// namespace VK4 {
// 	class Vk_TextureBuffer {
// 	public:
// 		Vk_TextureBuffer(
// 			Vk_Device* const device,
// 			Vk_SamplingResolution* samplingResolution
// 		)
// 			:
// 			_device(device),
// 			_samplingResolution(samplingResolution)
// 		{
// 			VK3::Vk_Logger::Log(typeid(this), "Create Texture Buffer (empty!)");
// 		}

// 		~Vk_TextureBuffer() {
// 			VK3::Vk_Logger::Log(typeid(this), "Destroy Texture Buffer (empty!)");
// 		}

// 		VkImageView vk_textureImageView() {
// 			return _textureImageView;
// 		}

// 	private:
// 		Vk_Device* _device;
// 		Vk_SamplingResolution* _samplingResolution;

// 		VkImage _textureImage;
// 		VkDeviceMemory _textureImageMemory;
// 		VkImageView _textureImageView;
// 	};
// }