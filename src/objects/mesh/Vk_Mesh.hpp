#pragma once

#include "../I_Object.hpp"

namespace VK4 {

	namespace Vk_Mesh_Structures {
		typedef struct Vk_PushConstants {
			float alpha;
			float pointSize;
		} Vk_PushConstants;
	}

	static std::string meshObjectTypes() {
		std::string tab = "\t";
		std::string nl = "\n";
		return 
			tab + GlobalCasters::castHighlightYellow(typeid(ObjectType_P_C).name()) + nl +
			tab + GlobalCasters::castHighlightYellow(typeid(ObjectType_Info).name()) + nl;
	}

	template<class T>
	class Vk_Mesh : I_Object<T> {
	public:

		Vk_Mesh() : I_Object<ObjectType_Info>(nullptr, "None", "None", "Vk_Mesh<T>", VK4::Topology::Triangles, VK4::CullMode::NoCulling, VK4::RenderType::Wireframe) {
			Vk_Logger::RuntimeError(typeid(this), 
				GlobalCasters::castRed("Attempt to instantiate Vk_Mesh with invalid template format ") + 
				GlobalCasters::castHighlightRed(typeid(T).name()) + 
				std::string("!\n") +
				GlobalCasters::castRed("Possible types are:\n") +
				dotObjectTypes()
			);
		}

		void vk_00_bindPipeline(const Vk_BindingProperties& props) override {}
		void vk_10_bindLineWidth(const Vk_BindingProperties& props) override {}
		void vk_20_bindDataBuffer(const Vk_BindingProperties& props) override {}
		void vk_30_bindPushConstants(const Vk_BindingProperties& props) override {}
		void vk_40_bindUniformBuffers(const Vk_BindingProperties& props) override {}
		void vk_50_bindDescriptorSets(const Vk_BindingProperties& props) override {}
		void vk_99_bindFinalize(const Vk_BindingProperties& props) override {}
	};

	template<> class Vk_Mesh<ObjectType_Info> : I_Object<ObjectType_Info> {
	public:
		Vk_Mesh() : I_Object<ObjectType_Info>(nullptr, "None", "None", glm::tmat3x3<point_type>(), "Vk_Mesh<ObjectType_Info>", VK4::Topology::Triangles, VK4::CullMode::NoCulling, VK4::RenderType::Wireframe) {
			Vk_Logger::Message(typeid(this), "Valid ObjectTypes:\n" + meshObjectTypes());
		}

		void vk_00_bindPipeline(const Vk_BindingProperties& props) override {}
		void vk_10_bindLineWidth(const Vk_BindingProperties& props) override {}
		void vk_20_bindDataBuffer(const Vk_BindingProperties& props) override {}
		void vk_30_bindPushConstants(const Vk_BindingProperties& props) override {}
		void vk_40_bindUniformBuffers(const Vk_BindingProperties& props) override {}
		void vk_50_bindDescriptorSets(const Vk_BindingProperties& props) override {}
		void vk_99_bindFinalize(const Vk_BindingProperties& props) override {}
	};

	template<> 
	class Vk_Mesh<ObjectType_P_C> : public I_Object<ObjectType_P_C> {
	public:

		Vk_Mesh(
			Vk_Device* const device,
			std::string objectName,
			std::string shaderName,
			glm::tmat4x4<point_type> modelMatrix,
			const Vk_Vertex_P* p,
			size_t pLen,
			const Vk_Vertex_C* c,
			size_t cLen,
			const index_type* indices,
			size_t iLen,
			// Topology topology,
			CullMode cullMode,
			RenderType renderType,
			float pointSize,
			float lineWidth,
			float alpha,
			std::unordered_map<std::string, int> bindingPoints,
			Vk_BufferUpdateBehaviour updateBehaviour,
			Vk_BufferSizeBehaviour sizeBehaviour
		)
			:
			I_Object<ObjectType_P_C>(device, objectName, shaderName, modelMatrix, "Vk_Mesh<ObjectType_P_C>", Topology::Triangles, cullMode, renderType),
			_iBuffer(nullptr),
			_vBuffer(nullptr),
			_cBuffer(nullptr),
			_pointSize(pointSize),
			_lineWidth(lineWidth),
			_alpha(alpha),
			_bindingPoints(bindingPoints)
		{
			_iBuffer = std::make_unique<Vk_DataBuffer<index_type>>(
				device, objectName, indices, iLen, updateBehaviour, sizeBehaviour, _objectType.type() + "_index-buffer"
			);

			_vBuffer = std::make_unique<Vk_DataBuffer<Vk_Vertex_P>>(
				device, objectName, p, pLen, updateBehaviour, sizeBehaviour, _objectType.type() + "_position-buffer"
			);

			_cBuffer = std::make_unique<Vk_DataBuffer<Vk_Vertex_C>>(
				device, objectName, c, cLen, updateBehaviour, sizeBehaviour, _objectType.type() + "_color-buffer"
			);
		}

		~Vk_Mesh(){}


// ############################################################################################################
//                       █     █ ██████  ██████     █    ███████ ███████ ██████   █████                        
//                       █     █ █     █ █     █   █ █      █    █       █     █ █     █                       
//                       █     █ █     █ █     █  █   █     █    █       █     █ █                             
//                       █     █ ██████  █     █ █     █    █    █████   ██████   █████                        
//                       █     █ █       █     █ ███████    █    █       █   █         █                       
//                       █     █ █       █     █ █     █    █    █       █    █  █     █                       
//                        █████  █       ██████  █     █    █    ███████ █     █  █████                        
// ############################################################################################################
		void vk_updatePoints(
			const std::vector<point_type>& points,
			size_t newFrom
		){
			if(!(points.size()%Vk_Vertex_P::innerDimensionLen() == 0)){
				Vk_Logger::RuntimeError(typeid(NoneObj), "Vertices size must be a multiple of {0} but is {1}", Vk_Vertex_P::innerDimensionLen(), points.size());
			}

			size_t pLen = static_cast<size_t>(points.size() / Vk_Vertex_P::innerDimensionLen());
			const Vk_Vertex_P* p = reinterpret_cast<const Vk_Vertex_P*>(points.data());
			if(newFrom >= pLen){
				Vk_Logger::RuntimeError(typeid(this), "'newFrom' must be smaller than 'newCount' but newFrom={0} and newCount={0}", newFrom, pLen);
			}
			
			_vBuffer->vk_update(p, pLen, newFrom);
		}

#ifndef PYVK // need to avoid methods with the same name and we want to export the one with the regular vector
		void vk_updatePoints(
			const std::vector<Vk_Vertex_P>& points,
			size_t newFrom
		){
			size_t pLen = points.size();
			const Vk_Vertex_P* p = points.data();

			if(newFrom >= pLen){
				Vk_Logger::RuntimeError(typeid(this), "'newFrom' must be smaller than 'newCount' but newFrom={0} and newCount={0}", newFrom, pLen);
			}
			
			_vBuffer->vk_update(p, pLen, newFrom);
		}
#endif

#ifdef PYVK
		void vk_update_points(
			const py::array_t<VK4::point_type, py::array::c_style>& points,
			size_t newFrom
		){
			size_t pLen;
			Vk_Vertex_P* p = Vk_NumpyTransformers::structArrayToCpp<Vk_Vertex_P>(points, pLen);

			if(newFrom >= pLen){
				Vk_Logger::RuntimeError(typeid(this), "'newFrom' must be smaller than 'newCount' but newFrom={0} and newCount={0}", newFrom, pLen);
			}
			
			_vBuffer->vk_update(p, pLen, newFrom);
		}
#endif




		void vk_updateColors(
			const std::vector<point_type>& colors,
			size_t newFrom
		){
			if(!(colors.size()%Vk_Vertex_C::innerDimensionLen() == 0)){
				Vk_Logger::RuntimeError(typeid(NoneObj), "Colors size must be a multiple of {0} but is {1}", Vk_Vertex_C::innerDimensionLen(), colors.size());

			}

			size_t cLen = static_cast<size_t>(colors.size() / Vk_Vertex_C::innerDimensionLen());
			const Vk_Vertex_C* c = reinterpret_cast<const Vk_Vertex_C*>(colors.data());

			if(newFrom >= cLen){
				Vk_Logger::RuntimeError(typeid(this), "'newFrom' must be smaller than 'newCount' but newFrom={0} and newCount={0}", newFrom, cLen);
			}
			
			_cBuffer->vk_update(c, cLen, newFrom);
		}

#ifndef PYVK // need to avoid methods with the same name and we want to export the one with the regular vector
		void vk_updateColors(
			const std::vector<Vk_Vertex_C>& colors,
			size_t newFrom
		){
			size_t cLen = colors.size();
			const Vk_Vertex_C* c = colors.data();

			if(newFrom >= cLen){
				Vk_Logger::RuntimeError(typeid(this), "'newFrom' must be smaller than 'newCount' but newFrom={0} and newCount={0}", newFrom, cLen);
			}
			
			_cBuffer->vk_update(c, cLen, newFrom);
		}
#endif

#ifdef PYVK
		void vk_update_colors(
			const py::array_t<VK4::point_type, py::array::c_style>& colors,
			size_t newFrom
		){
			size_t cLen;
			Vk_Vertex_C* c = Vk_NumpyTransformers::structArrayToCpp<Vk_Vertex_C>(colors, cLen);

			if(newFrom >= cLen){
				Vk_Logger::RuntimeError(typeid(this), "'newFrom' must be smaller than 'newCount' but newFrom={0} and newCount={0}", newFrom, cLen);
			}
			
			_cBuffer->vk_update(c, cLen, newFrom);
		}
#endif




		void vk_updateIndces(
			const std::vector<index_type>& indices,
			size_t newFrom
		){
			size_t iLen = indices.size();
			const index_type* i = indices.data();
			if(newFrom >= iLen){
				Vk_Logger::RuntimeError(typeid(this), "'newFrom' must be smaller than 'newCount' but newFrom={0} and newCount={0}", newFrom, iLen);
			}
			
			_iBuffer->vk_update(i, iLen, newFrom);
		}

#ifdef PYVK
		void vk_update_indices(
			const py::array_t<VK4::index_type, py::array::c_style>& indices,
			size_t newFrom
		){
			size_t iLen;
			index_type* i = Vk_NumpyTransformers::indexArrayToCpp(indices, iLen);
			if(newFrom >= iLen){
				Vk_Logger::RuntimeError(typeid(this), "'newFrom' must be smaller than 'newCount' but newFrom={0} and newCount={0}", newFrom, iLen);
			}
			
			_iBuffer->vk_update(i, iLen, newFrom);
		}
#endif




		void vk_updateAlpha(float alpha){
			_alpha = alpha;
		}

		void vk_updatePointSize(float pointSize){
			_pointSize = pointSize;
		}

		void vk_updateLineWidth(float lineWidth){
			_lineWidth = lineWidth;
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
		void vk_00_bindPipeline(const Vk_BindingProperties& props) override {
			checkAttached(props.viewportId);
			vkCmdBindPipeline(props.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline.at(props.viewportId)->vk_pipeline());
		}

		void vk_10_bindLineWidth(const Vk_BindingProperties& props) override {
			checkAttached(props.viewportId);

			vkCmdSetLineWidth(props.commandBuffer, _lineWidth);
		}

		void vk_20_bindDataBuffer(const Vk_BindingProperties& props) override {
			checkAttached(props.viewportId);

			VkBuffer vertexBuffers[] = { _vBuffer->vk_buffer() };
			std::uint32_t vertexBindingPoint = _bindingPoints.at("P_BindingPoint");
			VkDeviceSize vertexOffsets[] = { 0 };
			vkCmdBindVertexBuffers(
				props.commandBuffer,
				vertexBindingPoint,
				1,
				vertexBuffers,
				vertexOffsets
			);

			VkBuffer colorBuffers[] = { _cBuffer->vk_buffer() };
			std::uint32_t colorBindingPoint = _bindingPoints.at("C_BindingPoint");
			VkDeviceSize colorOffsets[] = { 0 };
			vkCmdBindVertexBuffers(
				props.commandBuffer,
				colorBindingPoint,
				1,
				colorBuffers,
				colorOffsets
			);

			VkBuffer indexBuffer = _iBuffer->vk_buffer();
			vkCmdBindIndexBuffer(
				props.commandBuffer,
				indexBuffer,
				0,
				VK_INDEX_TYPE_UINT32
			);
		}

		void vk_30_bindPushConstants(const Vk_BindingProperties& props) override {
			checkAttached(props.viewportId);

			Vk_Mesh_Structures::Vk_PushConstants constants = {};
			// if we draw points, we use this as point size, otherwise it doesn't matter
			constants.alpha = _alpha;
			constants.pointSize = _pointSize;

			vkCmdPushConstants(
				props.commandBuffer,
				_pipeline.at(props.viewportId)->vk_pipelineLayout(),
				VK_SHADER_STAGE_VERTEX_BIT,
				0,
				sizeof(Vk_Mesh_Structures::Vk_PushConstants),
				&constants
			);
		}

		void vk_40_bindUniformBuffers(const Vk_BindingProperties& props) override {
			// const auto caps = props.capabilities;
			// int nFramesInFlight = caps.nFramesInFlight;
			auto lDev = _device->vk_lDev();

			//std::vector<VkWriteDescriptorSet> descriptorWrites;
			if(props.uniformBuffers.find(1) != props.uniformBuffers.end()){
				Vk_Logger::RuntimeError(typeid(this), "Vk_Mesh requires binding at 1 but it's already taken");
			}

			// iterate all uniform buffers
			auto& sets = _sets[props.viewportId];
			for (auto ub : props.uniformBuffers) {
				// assign each uniform buffer to a descriptor set

				VkDescriptorBufferInfo bufferInfo{
					.buffer = ub.second->vk_uniformBuffer(props.frameInFlightIndex),
					.offset = 0,
					.range = ub.second->vk_sizeofBuffer()
				};

				VkWriteDescriptorSet set{};
				set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				set.dstSet = sets[props.frameInFlightIndex];
				set.dstBinding = ub.first;
				set.dstArrayElement = 0;
				set.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				set.descriptorCount = 1;
				set.pBufferInfo = &bufferInfo; //.data();

				vkUpdateDescriptorSets(
					lDev,
					1,
					&set,
					0,
					nullptr
				);
			}

			// bind obj local buffer
			VkDescriptorBufferInfo bufferInfo{
				.buffer = _uBuffer->vk_uniformBuffer(props.frameInFlightIndex),
				.offset = 0,
				.range = _uBuffer->vk_sizeofBuffer()
			};

			VkWriteDescriptorSet set{};
			set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			set.dstSet = sets[props.frameInFlightIndex];
			set.dstBinding = 1;
			set.dstArrayElement = 0;
			set.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			set.descriptorCount = 1;
			set.pBufferInfo = &bufferInfo; //.data();

			vkUpdateDescriptorSets(
				lDev,
				1,
				&set,
				0,
				nullptr
			);
		}

		void vk_50_bindDescriptorSets(const Vk_BindingProperties& props) override {
			checkAttached(props.viewportId);

			vkCmdBindDescriptorSets(
				props.commandBuffer,
				VK_PIPELINE_BIND_POINT_GRAPHICS,
				_pipeline.at(props.viewportId)->vk_pipelineLayout(),
				0,
				1,
				&(_sets[props.viewportId][props.frameInFlightIndex]),
				0,
				nullptr
			);
		}

		void vk_99_bindFinalize(const Vk_BindingProperties& props) override {
			vkCmdDrawIndexed(
				props.commandBuffer,
				static_cast<std::uint32_t>(_iBuffer->vk_bufferCount()),
				1, 0, 0, 0);
		}


// ############################################################################################################
//                         ██████  ██████  ███ █     █    █    ███████ ███████  █████                          
//                         █     █ █     █  █  █     █   █ █      █    █       █     █                         
//                         █     █ █     █  █  █     █  █   █     █    █       █                               
//                         ██████  ██████   █  █     █ █     █    █    █████    █████                          
//                         █       █   █    █   █   █  ███████    █    █             █                         
//                         █       █    █   █    █ █   █     █    █    █       █     █                         
//                         █       █     █ ███    █    █     █    █    ███████  █████                          
// ############################################################################################################
	private:
		std::unique_ptr<Vk_DataBuffer<index_type>> _iBuffer;
		std::unique_ptr<Vk_DataBuffer<Vk_Vertex_P>> _vBuffer;
		std::unique_ptr<Vk_DataBuffer<Vk_Vertex_C>> _cBuffer;
		
		float _pointSize;
		float _lineWidth;
		float _alpha;
		std::unordered_map<std::string, int> _bindingPoints;

		void checkAttached(LWWS::TViewportId viewportId) {
			if (_pipeline.find(viewportId) == _pipeline.end()) Vk_Logger::RuntimeError(typeid(this), "Trying to bind unattached " + type());
		}
	};
}