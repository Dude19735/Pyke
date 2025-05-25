#pragma once

// #include <vulkan/vulkan.h>
#include <chrono>
#include "../../Defines.h"
#include "Vk_Mesh.hpp"

namespace VK4 {
	class S_Mesh_P_C {
	public:

		S_Mesh_P_C() {}
		
		static inline const int Vertex_P_BindingPoint = 0;
		static inline const int Vertex_P_BindingLocation = 0;
		static inline const int Vertex_C_BindingPoint = 1;
		static inline const int Vertex_C_BindingLocation = 1;

		static inline const std::string Identifier = "s_mesh_p_c";

		static const Vk_Config_GraphicsPipeline_IM getPipelineConfig(
			I_Renderer* renderer,
			CullMode cullMode,
			RenderType renderType
		) {
			std::vector<VkVertexInputBindingDescription> bindingDescription = {
				Vk_Vertex_P::getBindingDescription(Vertex_P_BindingPoint),
				Vk_Vertex_C::getBindingDescription(Vertex_C_BindingPoint)
			};
			std::vector<VkVertexInputAttributeDescription> attributeDescriptions = {
				Vk_Vertex_P::getAttributeDescriptions(Vertex_P_BindingPoint, Vertex_P_BindingLocation),
				Vk_Vertex_C::getAttributeDescriptions(Vertex_C_BindingPoint, Vertex_C_BindingLocation)
			};

			const Vk_Config_GraphicsPipeline_IM res{
				.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
				.sizeofPushConstants = sizeof(Vk_Mesh_Structures::Vk_PushConstants),
				.cullMode = cullMode,
				.renderType = renderType,
				.vertexShader = renderer->vk_getOrCreate_VertexShader(Identifier),
				.fragmentShader = renderer->vk_getOrCreate_FragmentShader(Identifier),
				.renderPass = renderer->vk_pipelineAuxilliaries().renderpass,
				.descriptorSetLayout = renderer->vk_getOrCreate_DescriptorSetLayout(Identifier),
				.bindingDescription = bindingDescription,
				.attributeDescriptions = attributeDescriptions
			};

			return res;
		}

		static std::shared_ptr<Vk_Mesh<ObjectType_P_C>> create(
			Vk_Device* const device,
			std::string name,
			const std::vector<point_type>& modelMatrix,
			const std::vector<point_type>& p,
			const std::vector<point_type>& c,
			const std::vector<index_type>& i,
			// Topology topology = VK4::Topology::Points,
			float alpha=1.0f,
			CullMode cullMode = VK4::CullMode::Back,
			RenderType renderType = VK4::RenderType::Solid,
			float pointSize=1.0f,
			float lineWidth=1.0f,
			Vk_BufferUpdateBehaviour sizeBehaviour = Vk_BufferUpdateBehaviour::GlobalLock,
			Vk_BufferSizeBehaviour updateBehaviour = Vk_BufferSizeBehaviour::Init_1_0_Grow_1_5
		) {
			return S_Mesh_P_C::create(
				device, name,
				std::span<const point_type>(modelMatrix.data(), modelMatrix.size()),
				std::span<const point_type>(p.data(), p.size()),
				std::span<const point_type>(c.data(), c.size()),
				std::span<const index_type>(i.data(), i.size()),
				alpha, cullMode, renderType, pointSize, lineWidth, sizeBehaviour, updateBehaviour
			);
		}

		static std::shared_ptr<Vk_Mesh<ObjectType_P_C>> create(
			Vk_Device* const device,
			std::string name,
			const std::span<const point_type>& modelMatrix,
			const std::span<const point_type>& p,
			const std::span<const point_type>& c,
			const std::span<const index_type>& i,
			// Topology topology = VK4::Topology::Points,
			float alpha=1.0f,
			CullMode cullMode = VK4::CullMode::Back,
			RenderType renderType = VK4::RenderType::Solid,
			float pointSize=1.0f,
			float lineWidth=1.0f,
			Vk_BufferUpdateBehaviour sizeBehaviour = Vk_BufferUpdateBehaviour::GlobalLock,
			Vk_BufferSizeBehaviour updateBehaviour = Vk_BufferSizeBehaviour::Init_1_0_Grow_1_5
		) {
			if(modelMatrix.size() != 16){
				Vk_Logger::RuntimeError(typeid(NoneObj), "Size of model matrix must be 16 but is {0}", modelMatrix.size());
			}
			if(!(p.size()%Vk_Vertex_P::innerDimensionLen() == 0)){
				Vk_Logger::RuntimeError(typeid(NoneObj), "Vertices size must be a multiple of {0} but is {1}", Vk_Vertex_P::innerDimensionLen(), p.size());
			}
			if(!(c.size()%Vk_Vertex_C::innerDimensionLen() == 0)){
				Vk_Logger::RuntimeError(typeid(NoneObj), "Colors size must be a multiple of {0} but is C={2}", Vk_Vertex_C::innerDimensionLen(), c.size());
			}
			if(lineWidth != 1.0f && renderType != RenderType::Wireframe){
				Vk_Logger::Warn(typeid(NoneObj), "Mesh object only supports dynamic line width if renderType=Wireframe");
			}
			if(pointSize != 1.0f && renderType != RenderType::Point){
				Vk_Logger::Warn(typeid(NoneObj), "Mesh object only supports dynamic point size if renderType=Point");
			}

			auto obj = std::make_shared<Vk_Mesh<ObjectType_P_C>>(
				device,
				name,
				Identifier,
				glm::make_mat4x4<point_type>(modelMatrix.data()),
				reinterpret_cast<const Vk_Vertex_P*>(p.data()), static_cast<int>(p.size()/Vk_Vertex_P::innerDimensionLen()), 
				reinterpret_cast<const Vk_Vertex_C*>(c.data()), static_cast<int>(c.size()/Vk_Vertex_C::innerDimensionLen()), 
				i.data(), i.size(),
				cullMode,
				renderType,
				pointSize, lineWidth, alpha,
				std::unordered_map<std::string, int>{ {"P_BindingPoint", Vertex_P_BindingPoint}, {"C_BindingPoint", Vertex_C_BindingPoint} },
				sizeBehaviour,
				updateBehaviour
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
			sizes[0].descriptorCount = std::max(freshPoolSize * static_cast<int>(sizes.size()), 2); // just at least two because we have two uniform buffers

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
		}
	};
}