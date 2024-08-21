#pragma once

// #include <vulkan/vulkan.h>
#include <chrono>
#include "../../Defines.h"
#include "Vk_Dot.hpp"

namespace VK4 {
	class S_Dot_P_C {
	public:

		S_Dot_P_C() {}
		
		static inline const int Vertex_P_BindingPoint = 0;
		static inline const int Vertex_P_BindingLocation = 0;
		static inline const int Vertex_C_BindingPoint = 1;
		static inline const int Vertex_C_BindingLocation = 1;

		static inline const std::string Identifier = "s_dot_p_c";
			//(static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(
			//	std::chrono::system_clock::now().time_since_epoch()).count()
			//	) << 50
			//) >> 50;

		static const Vk_Config_GraphicsPipeline_IM getPipelineConfig(
			I_Renderer* renderer,
			// Topology topology,
			CullMode cullMode
			// RenderType renderType
		) {
			std::vector<VkVertexInputBindingDescription> bindingDescription = {
				VK4::Vk_Vertex_P::getBindingDescription(Vertex_P_BindingPoint),
				VK4::Vk_Vertex_C::getBindingDescription(Vertex_C_BindingPoint)
			};
			std::vector<VkVertexInputAttributeDescription> attributeDescriptions = {
				VK4::Vk_Vertex_P::getAttributeDescriptions(Vertex_P_BindingPoint, Vertex_P_BindingLocation),
				VK4::Vk_Vertex_C::getAttributeDescriptions(Vertex_C_BindingPoint, Vertex_C_BindingLocation)
			};

			// VkPrimitiveTopology topo = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
			// if (topology == Topology::Triangles) topo = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
			// else if (topology == Topology::Triangles) topo = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;

			const Vk_Config_GraphicsPipeline_IM res{
				.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST,
				.sizeofPushConstants = sizeof(Vk_Dot_Structures::Vk_PushConstants),
				.cullMode = cullMode,
				.renderType = RenderType::Point,
				.vertexShader = renderer->vk_getOrCreate_VertexShader(Identifier),
				.fragmentShader = renderer->vk_getOrCreate_FragmentShader(Identifier),
				.renderPass = renderer->vk_pipelineAuxilliaries().renderpass,
				.descriptorSetLayout = renderer->vk_getOrCreate_DescriptorSetLayout(Identifier),
				.bindingDescription = bindingDescription,
				.attributeDescriptions = attributeDescriptions
			};

			return res;
		}

#ifndef PYVK
		static std::shared_ptr<Vk_Dot<ObjectType_P_C>> create(
			Vk_Device* const device,
			std::string name,
			const std::array<point_type, 16>& modelMatrix,
			const std::vector<point_type>& p,
			const std::vector<point_type>& c,
			const std::vector<index_type>& i,
			float pointSize,
			float alpha,
			// Topology topology = VK4::Topology::Points,
			CullMode cullMode = VK4::CullMode::NoCulling,
			// RenderType renderType = VK4::RenderType::Point,
			Vk_BufferUpdateBehaviour updateBehaviour = Vk_BufferUpdateBehaviour::GlobalLock,
			Vk_BufferSizeBehaviour sizeBehaviour = Vk_BufferSizeBehaviour::Init_1_0_Grow_1_5
		){
			if(!(p.size() == c.size() && p.size() == 3*i.size())){
				Vk_Logger::RuntimeError(typeid(NoneObj), "Vertices and Colors sizes must be 3 times larger than Index size but sizes are V={0}, C={1}, I={2}", p.size(), c.size(), i.size());
			}
			if(!(p.size()%Vk_Vertex_P::innerDimensionLen() == 0)){
				Vk_Logger::RuntimeError(typeid(NoneObj), "Vertices size must be a multiple of {0} but is {1}", Vk_Vertex_P::innerDimensionLen(), p.size());
			}

			if(!(c.size()%Vk_Vertex_C::innerDimensionLen() == 0)){
				Vk_Logger::RuntimeError(typeid(NoneObj), "Colors size must be a multiple of {0} but is C={2}", Vk_Vertex_C::innerDimensionLen(), c.size());
			}

			auto obj = std::make_shared<Vk_Dot<ObjectType_P_C>>(
				device,
				name,
				Identifier,
				glm::make_mat4x4<point_type>(modelMatrix.data()), 
				reinterpret_cast<const Vk_Vertex_P*>(p.data()), static_cast<int>(p.size()/Vk_Vertex_P::innerDimensionLen()), 
				reinterpret_cast<const Vk_Vertex_C*>(c.data()), static_cast<int>(c.size()/Vk_Vertex_C::innerDimensionLen()), 
				i.data(), i.size(),
				pointSize, alpha,
				cullMode,
				std::unordered_map<std::string, int>{ {"P_BindingPoint", Vertex_P_BindingPoint}, {"C_BindingPoint", Vertex_C_BindingPoint} },
				updateBehaviour,
				sizeBehaviour
			);

			return obj;
		}
#endif

		static std::shared_ptr<Vk_Dot<ObjectType_P_C>> create(
			Vk_Device* const device,
			std::string name,
#ifdef PYVK
			const py::array_t<VK4::point_type, py::array::c_style>& modelMatrix,
			const py::array_t<VK4::point_type, py::array::c_style>& points,
			const py::array_t<VK4::point_type, py::array::c_style>& colors,
			const py::array_t<VK4::index_type, py::array::c_style>& indices,
#else
			const glm::tmat4x4<point_type>& modelMatrix,
			const std::vector<Vk_Vertex_P>& p,
			const std::vector<Vk_Vertex_C>& c,
			const std::vector<index_type>& i,
#endif
			float pointSize,
			float alpha,
			// Topology topology = VK4::Topology::Points,
			CullMode cullMode = VK4::CullMode::NoCulling,
			// RenderType renderType = VK4::RenderType::Point,
			Vk_BufferUpdateBehaviour updateBehaviour = Vk_BufferUpdateBehaviour::GlobalLock,
			Vk_BufferSizeBehaviour sizeBehaviour = Vk_BufferSizeBehaviour::Init_1_0_Grow_1_5
		) {
#ifdef PYVK
			glm::tmat4x4<point_type> m = Vk_NumpyTransformers::arrayToGLM4x4<point_type>(modelMatrix);
			size_t pLen;
			Vk_Vertex_P* p = Vk_NumpyTransformers::structArrayToCpp<Vk_Vertex_P>(points, pLen);
			size_t cLen;
			Vk_Vertex_C* c = Vk_NumpyTransformers::structArrayToCpp<Vk_Vertex_C>(colors, cLen);
			size_t iLen;
			index_type* i = Vk_NumpyTransformers::indexArrayToCpp(indices, iLen);
#endif
			auto obj = std::make_shared<Vk_Dot<ObjectType_P_C>>(
				device,
				name,
				Identifier,
#ifdef PYVK
				m, p, pLen, c, cLen, i, iLen,
#else
				modelMatrix, p.data(), p.size(), c.data(), c.size(), i.data(), i.size(), 
#endif
				pointSize, alpha,
				cullMode,
				std::unordered_map<std::string, int>{ {"P_BindingPoint", Vertex_P_BindingPoint}, {"C_BindingPoint", Vertex_C_BindingPoint} },
				updateBehaviour,
				sizeBehaviour
			);

			return obj;
		}

		static VkDescriptorSetLayout createDescriptorSetLayout(VkDevice lDev) {
			std::vector<VkDescriptorSetLayoutBinding> bindings;

			VkDescriptorSetLayoutBinding rendererUbLayoutBinding{};
			rendererUbLayoutBinding.binding = 0; // UniformBufferObject
			rendererUbLayoutBinding.descriptorCount = 1; // > 1 => array
			rendererUbLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			rendererUbLayoutBinding.pImmutableSamplers = nullptr;
			rendererUbLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

			VkDescriptorSetLayoutBinding modelUbLayoutBinding{};
			modelUbLayoutBinding.binding = 1; // VpUniformBufferObject
			modelUbLayoutBinding.descriptorCount = 1; // > 1 => array
			modelUbLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			modelUbLayoutBinding.pImmutableSamplers = nullptr;
			modelUbLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

			bindings.push_back(rendererUbLayoutBinding);
			bindings.push_back(modelUbLayoutBinding);

			//std::vector<ShaderLayout::UniformBuffer_FragmentShader> ubf = *_layoutDescriptor->vk_ubf();
			//for (auto b : ubf) {
			//	VkDescriptorSetLayoutBinding uboLayoutBinding{};
			//	uboLayoutBinding.binding = b.bindingPoint;
			//	uboLayoutBinding.descriptorCount = b.descriptorCount;
			//	uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			//	uboLayoutBinding.pImmutableSamplers = nullptr;
			//	uboLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

			//	bindings.push_back(uboLayoutBinding);
			//}

			//VkDescriptorSetLayoutBinding samplerLayoutBinding{};
			//samplerLayoutBinding.binding = bindingPoint;
			//samplerLayoutBinding.descriptorCount = 1;
			//samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			//samplerLayoutBinding.pImmutableSamplers = nullptr;
			//samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

			//bindings.push_back(samplerLayoutBinding);
			//bindingPoint++;

			VkDescriptorSetLayoutCreateInfo layoutInfo{};
			layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
			layoutInfo.pBindings = bindings.data();

			VkDescriptorSetLayout layout;
			VK_CHECK(
				vkCreateDescriptorSetLayout(
					lDev,
					&layoutInfo,
					nullptr,
					&layout),
				"Failed to create descriptor set layout!");

			return layout;
			//_descriptorSetLayout[Dot_P_C] = layout;
		}

		static VkDescriptorPool createDescriptorPool(VkDevice lDev, int freshPoolSize) {
			if(freshPoolSize == 0){
				Vk_Logger::RuntimeError(typeid(NoneObj), "[createDescriptorPool]: freshPoolSize > 0 required but is {0}!", freshPoolSize);
			}
			std::array<VkDescriptorPoolSize, 1> sizes{};
			sizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; // uniform buffer
			// this is the total amount of descriptors
			//  - one uniform buffer == one descriptor
			//  - two uniform buffers == two descriptors
			//  - ...
			sizes[0].descriptorCount = std::max(freshPoolSize * static_cast<int>(sizes.size()), 2);

			VkDescriptorPoolCreateInfo poolInfo{};
			poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			poolInfo.poolSizeCount = static_cast<uint32_t>(sizes.size());
			poolInfo.pPoolSizes = sizes.data();
			// this is the total amount of sets
			//  - the shader program has exactly one uniform buffer => maxSets == descriptorCount
			//  - the shader program has exactly two uniform buffers => 2 * maxSets == descriptorCount
			//  - the shader program has exactly three uniform buffers => 3 * maxSets = descriptorCount
			//  - ...
			poolInfo.maxSets = freshPoolSize; // one set per shader and framebuffer

			VkDescriptorPool pool;
			VK_CHECK(
				vkCreateDescriptorPool(
					lDev,
					&poolInfo,
					nullptr,
					&pool),
				"Failed to create descriptor pool!");

			// create or add a fresh pool
			return pool;
			//_descriptorPools[Dot_P_C].push_back(DescriptorPool{ .remaining = _freshPoolSize, .pool = pool });
		}

		static std::vector<VkDescriptorSet> createDescriptorSets(
			VkDevice lDev, 
			VkDescriptorSetLayout layout,
			VkDescriptorPool pool, 
			int count
		) {
			std::vector<VkDescriptorSetLayout> layouts(
				count,
				layout
			);

			VkDescriptorSetAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			allocInfo.descriptorPool = pool;
			allocInfo.descriptorSetCount = count;
			allocInfo.pSetLayouts = layouts.data();

			std::vector<VkDescriptorSet> sets(count);

			VkResult res = vkAllocateDescriptorSets(lDev, &allocInfo, sets.data());
			VK_CHECK(res, "Failed to allocate descriptor sets!");

			// return the ones created in this go for the assigned object to use
			return sets;

			// this one has to go somewhere else!!!!!!!!!!!!!!
			//std::vector<VkDescriptorBufferInfo> bufferInfo;
			//std::vector<VkWriteDescriptorSet> descriptorWrites;
			//for (size_t i = 0; i < _nFramesInFlight; i++) {
			//	
			//	VkDescriptorBufferInfo descriptorBufferInfo{};
			//	descriptorBufferInfo.offset = 0;
			//	descriptorBufferInfo.buffer = (*vkBuf->vk_uniformBuffers())[i];
			//	descriptorBufferInfo.range = vkBuf->vk_sizeofBuffer();

			//	addUniformBuffer(
			//		descriptorWrites,
			//		0,
			//		descriptorBufferInfo,
			//		_descriptorSets[i]
			//	);

			//	vkUpdateDescriptorSets(
			//		lDev,
			//		static_cast<uint32_t>(descriptorWrites.size()),
			//		descriptorWrites.data(),
			//		0,
			//		nullptr
			//	);
			//}
		}
	};
}