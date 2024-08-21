#pragma once

#include <string>
#include <fstream>
// #include <vulkan/vulkan.h>

#include "../Defines.h"
// #include "../Vk_Logger.hpp"
#include "../application/Vk_Device.hpp"
#include "Vk_ShaderBin"

namespace VK4 {

	//enum class ShadingType {
	//	None,
	//	Phong
	//};

	//static std::string ShadingTypeToString(ShadingType type) {
	//	switch (type) {
	//	case ShadingType::None:
	//		return "None";
	//	case ShadingType::Phong:
	//		return "Phong";
	//	default:
	//		return "Unknown";
	//	}
	//}

	class Vk_Shader {
	public:

		Vk_Shader(
			Vk_Device* const device,
			std::string shaderName
		)
			:
			_device(device),
			_shaderName(shaderName)
		{
			Vk_Logger::Log(typeid(this), GlobalCasters::castConstructorTitle(std::string("Create Shadermodule ") + shaderName));

			std::vector<unsigned char> buffer = ShaderBin::S[shaderName];
			VkShaderModuleCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			createInfo.codeSize = buffer.size();
			createInfo.pCode = reinterpret_cast<const uint32_t*>(buffer.data());

			VK_CHECK(vkCreateShaderModule(_device->vk_lDev(), &createInfo, nullptr, &_shaderModule), "Failed to create shader module!");
		}

		~Vk_Shader() {
			Vk_Logger::Log(typeid(this), GlobalCasters::castDestructorTitle(std::string("Destroy Shadermodule ") + _shaderName));
			vkDestroyShaderModule(_device->vk_lDev(), _shaderModule, nullptr);
		}

		const VkShaderModule vk_shaderModule() const {
			return _shaderModule;
		}



	private:
		Vk_Device* const _device;
		std::string _shaderName;

		VkShaderModule _shaderModule;
	};
}