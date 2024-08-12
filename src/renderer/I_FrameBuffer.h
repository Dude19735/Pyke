#pragma once

// #include <vulkan/vulkan.h>
#include "../Defines.h"

namespace VK4 {
	class I_FrameBuffer {
	public:
		virtual ~I_FrameBuffer(){}
		virtual std::vector<VkFramebuffer>* vk_frameBuffers() = 0;
	};
}