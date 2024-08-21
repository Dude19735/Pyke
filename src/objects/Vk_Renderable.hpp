#pragma once

// #include <vulkan/vulkan.h>
#include "../Defines.h"
#include "../buffers/Vk_UniformBuffer.hpp"

namespace VK4 {
	class Vk_RenderableTypeCaster;
	class I_GraphicsPipeline;

	struct Vk_BindingProperties {
		const SwapchainSupportDetails& capabilities;
		int camId;
		int frameInFlightIndex;
		VkCommandBuffer commandBuffer;
		std::unordered_map<int, Vk_AbstractUniformBuffer*> uniformBuffers;
	};

	class Vk_Renderable { //: public std::enable_shared_from_this<Vk_Renderable> {
	public:
		friend class I_Renderer;
		friend class Vk_RenderableTypeCaster;
#ifdef PYVK
		Vk_Renderable() 
			: 
			_device(nullptr),
			_shaderName("shaderName"),
			_objectName("objectName"),
			_typeName("typeName"),
			_modelMatrix(UniformBufferType_ModelMat4{ .mat = glm::zero<glm::mat4x4>() }),
			_topology(Topology::Points),
			_cullMode(CullMode::NoCulling),
			_renderType(RenderType::Point)
		{}
#endif

		Vk_Renderable(Vk_Device* const device, std::string objectName, std::string shaderName, glm::tmat4x4<point_type> modelMatrix, std::string typeName, Topology topology, CullMode cullMode, RenderType renderType) 
			: 
			_device(device),
			_shaderName(shaderName),
			_objectName(objectName),
			_typeName(typeName),
			_modelMatrix(UniformBufferType_ModelMat4{ .mat = modelMatrix }),
			_topology(topology),
			_cullMode(cullMode),
			_renderType(renderType)
		{
			Vk_Logger::Log(typeid(this), 
				GlobalCasters::castConstructorTitle(
					std::string("Create Vk_Renderable [") + _objectName + std::string("]")
				)
			);

			// auto caps = _device->vk_swapchainSupportActiveDevice(nullptr);
			_uBuffer = std::make_unique<Vk_UniformBuffer<UniformBufferType_ModelMat4>>(
				_device, 
				_objectName,
				10, //caps.nFramesInFlight,
				VK4::UniformBufferType_ModelMat4{ .mat = modelMatrix }
			);
		}

		virtual ~Vk_Renderable() {
			Vk_Logger::Log(typeid(this), 
				GlobalCasters::castDestructorTitle(
					std::string("Delete Vk_Renderable [") + _objectName + std::string("]")
				)
			);
		}


// ############################################################################################################
//                             ██████  ███ █     █ ██████  ███████ ██████   █████                              
//                             █     █  █  ██    █ █     █ █       █     █ █     █                             
//                             █     █  █  █ █   █ █     █ █       █     █ █                                   
//                             ██████   █  █  █  █ █     █ █████   ██████   █████                              
//                             █     █  █  █   █ █ █     █ █       █   █         █                             
//                             █     █  █  █    ██ █     █ █       █    █  █     █                             
//                             ██████  ███ █     █ ██████  ███████ █     █  █████                              
// ############################################################################################################
		virtual void vk_00_bindPipeline(const Vk_BindingProperties& props) {
			Vk_Logger::RuntimeError(typeid(this), "vk_bindPipeline not implemented");
		}
		virtual void vk_10_bindLineWidth(const Vk_BindingProperties& props) {
			Vk_Logger::RuntimeError(typeid(this), "vk_setLineWidth not implemented");
		}
		virtual void vk_20_bindDataBuffer(const Vk_BindingProperties& props) {
			Vk_Logger::RuntimeError(typeid(this), "vk_bindDataBuffer not implemented");
		}
		virtual void vk_30_bindPushConstants(const Vk_BindingProperties& props) {
			Vk_Logger::RuntimeError(typeid(this), "vk_pushConstants not implemented");
		}
		virtual void vk_40_bindUniformBuffers(const Vk_BindingProperties& props) {
			Vk_Logger::RuntimeError(typeid(this), "vk_bindUniformBuffers not implemented");
		}
		virtual void vk_50_bindDescriptorSets(const Vk_BindingProperties& props) {
			Vk_Logger::RuntimeError(typeid(this), "vk_bindDescriptorSets not implemented");
		}
		virtual void vk_99_bindFinalize(const Vk_BindingProperties& props) {
			Vk_Logger::RuntimeError(typeid(this), "vk_finalize not implemented");
		}

		const std::string& vk_objectName() { return _objectName; }
		const std::string& vk_shaderName() { return _shaderName; }
		const int vk_descriptorCount() { return static_cast<const int>(_sets.size()); }
		
		const glm::tmat4x4<point_type>& vk_modelMatrix() { 
			return _modelMatrix.mat; 
		}


// ############################################################################################################
//                       █     █ ██████  ██████     █    ███████ ███████ ██████   █████                        
//                       █     █ █     █ █     █   █ █      █    █       █     █ █     █                       
//                       █     █ █     █ █     █  █   █     █    █       █     █ █                             
//                       █     █ ██████  █     █ █     █    █    █████   ██████   █████                        
//                       █     █ █       █     █ ███████    █    █       █   █         █                       
//                       █     █ █       █     █ █     █    █    █       █    █  █     █                       
//                        █████  █       ██████  █     █    █    ███████ █     █  █████                        
// ############################################################################################################
#ifdef PYVK
		void vk_updateModelMatrix(py::array_t<VK4::point_type, py::array::c_style>& modelMatrix) {
			// py::buffer_info p_modelMatrix = modelMatrix.request();
			// VK4::point_type* p_modelMatrix_ptr = static_cast<VK4::point_type*>(p_modelMatrix.ptr);
			// glm::tmat4x4<point_type> m = glm::make_mat4x4(p_modelMatrix_ptr);
			_modelMatrix.mat = Vk_NumpyTransformers::arrayToGLM4x4<VK4::point_type>(modelMatrix);
#else
		void vk_updateModelMatrix(const glm::tmat4x4<point_type>& modelMatrix) { 
			_modelMatrix.mat = modelMatrix;
#endif

			//int s = _uBuffer->vk_frameCount(); // this is a reminder => don't use it, just remember where it comes from and remember that the uBuffer has 10 frames by default!!!
			int s = _device->bridge.nFrames();
			for(int i=0; i<s; ++i){
				_device->bridge.addUpdate(
					i, [this, i]() {
						_uBuffer->vk_update(static_cast<uint32_t>(i), static_cast<const void*>(&_modelMatrix));
					}
				);
			}
		}

		bool vk_isAttachedTo(int camId) {
			if (_pipeline.find(camId) == _pipeline.end()) return false;
			return true;
		}

		Vk_Device* vk_device() { return _device; }


// ############################################################################################################
//                   ██████  ██████  ███████ ███████ ███████  █████  ███████ ███████ ██████                    
//                   █     █ █     █ █     █    █    █       █     █    █    █       █     █                   
//                   █     █ █     █ █     █    █    █       █          █    █       █     █                   
//                   ██████  ██████  █     █    █    █████   █          █    █████   █     █                   
//                   █       █   █   █     █    █    █       █          █    █       █     █                   
//                   █       █    █  █     █    █    █       █     █    █    █       █     █                   
//                   █       █     █ ███████    █    ███████  █████     █    ███████ ██████                    
// ############################################################################################################
	protected:
		Vk_Device* const _device;
		const std::string _shaderName;
		const std::string _objectName;
		const std::string _typeName;
		UniformBufferType_ModelMat4 _modelMatrix;
		std::unique_ptr<Vk_UniformBuffer<UniformBufferType_ModelMat4>> _uBuffer;

		Topology _topology;
		CullMode _cullMode;
		RenderType _renderType;

		std::unordered_map<int, I_GraphicsPipeline*> _pipeline;
		std::unordered_map<int, std::vector<VkDescriptorSet>> _sets;
	};
}