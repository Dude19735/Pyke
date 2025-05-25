#pragma once

// #include <vulkan/vulkan.h>

#include <string>

#include "../Defines.h"
#include "Vk_Renderable.hpp"
#include "../application/Vk_Device.hpp"
#include "../renderer/I_Renderer.hpp"
#include "../buffers/Vk_DataBuffer.hpp"
#include "../buffers/Vk_UniformBuffer.hpp"
#include "Vk_ObjectType.hpp"

namespace VK4 {
	
	template<class T>
	class I_Object : public Vk_Renderable {
	public:
#ifdef PYVK
		I_Object() 
			: Vk_Renderable(nullptr, "objectName", "shaderName", glm::zero<glm::mat4x4>(), "typeName", Topology::Points, CullMode::NoCulling, RenderType::Point), _objectType("typeName")
		{}
#endif
		I_Object(Vk_Device* const device, std::string objectName, std::string shaderName, glm::tmat4x4<point_type> modelMatrix, std::string typeName, Topology topology, CullMode cullMode, RenderType renderType) 
			: Vk_Renderable(device, objectName, shaderName, modelMatrix, typeName, topology, cullMode, renderType), _objectType(typeName)
		{}

		virtual ~I_Object() {}

		std::string type() { return _objectType.type(); }

	protected:
		T _objectType;

	private:
	};
}
