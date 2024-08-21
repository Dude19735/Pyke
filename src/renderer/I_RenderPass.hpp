#pragma once

#include "../Defines.h"
// #include "../Vk_Logger.hpp"
#include "../application/Vk_Device.hpp"

namespace VK4 {
	class I_RenderPass {
	public:
		virtual ~I_RenderPass() {}
		virtual const VkRenderPass vk_renderPass() const = 0;
	private:
	};
}