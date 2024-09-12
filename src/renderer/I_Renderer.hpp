#pragma once

#include <array>
// #include <vulkan/vulkan.h>
#include "../Defines.h"
#include "../application/Vk_Device.hpp"
#include "../objects/Vk_Renderable.hpp"
#include "../objects/Vk_Shader.hpp"
#include "I_GraphicsPipeline.hpp"
#include "I_GraphicsPipelineConfig.hpp"

namespace VK4 {

	class I_Renderer {

	public:
		
		// struct Vk_PipelineAuxilliaries {
		// 	const Vk_Surface* surface;
		// 	const I_RenderPass* renderpass;
		// 	const I_Swapchain* swapchain;
		// 	const I_FrameBuffer* framebuffer;
		// };

		// https://herbsutter.com/2016/09/25/to-store-a-destructor/
		//struct Caster {
		//	void* p;
		//	void(*cast)(void*);
		//};

		I_Renderer(
			// LWWS::TViewportId viewportId, 
			Vk_Device* const device, 
			// Vk_PipelineAuxilliaries auxilliaries,
			const Vk_SurfaceConfig& surfaceConfig,
			const int freshPoolSize,
			UniformBufferType_RendererMat4 mvpInit)
			:
			//_cast(cast),
			_shaderPathPrefix(""),
			_ext_frag_spv(".frag.spv"),
			_ext_vert_spv(".vert.spv"),
			_device(device),
			// _pipelineAuxilliaries(auxilliaries),
			_freshPoolSize(freshPoolSize),
			_descriptorSetLayout({}),
			_descriptorPools({}),
			_descriptorSets({}),
			_pv(nullptr),
			_surfaceConfig(surfaceConfig)
		{
			VK4::Vk_Logger::Log(typeid(this), GlobalCasters::castConstructorTitle("Create I_Rasterizer (Baseclass)"));

			// create command buffers
			// allocate command buffers
// #ifdef PYVK
// 			py::gil_scoped_acquire acquire;
// 			py::object pyvkMod = py::module::import("pyvk");
//   			_shaderPathPrefix = pyvkMod.attr("__file__").cast<std::string>();
// 			auto location = _shaderPathPrefix.find("__init__.py");
// 			if(location != std::string::npos){
// 				_shaderPathPrefix = _shaderPathPrefix.substr(0, location) + "shaderprograms/";
// 			}
// 			py::gil_scoped_release nogil;
// #else
// 			_shaderPathPrefix = "shaderprograms/";
// #endif
			_shaderPathPrefix = ""; // don't use this in any other way!
			auto caps = _device->vk_swapchainSupportActiveDevice(_pipelineAuxilliaries.surface);

			// create mvp buffer
			std::string associatedObject = "I_Renderer_MPV";
			_pv = std::make_unique<Vk_UniformBuffer<UniformBufferType_RendererMat4>>(_device, associatedObject, caps.nFramesInFlight, mvpInit);
		}

		virtual ~I_Renderer() {
			// first call the "destructor" of the inheriting class
			//_iDestructor.destroy(_iDestructor.p);
			VK4::Vk_Logger::Log(typeid(this), GlobalCasters::castDestructorTitle("Destroy I_Renderer (Baseclass)"));

			_renderables.clear();

			const VkDevice lDev = _device->vk_lDev();

			for (auto dl : _descriptorSetLayout) {
				vkDestroyDescriptorSetLayout(lDev, dl.second, nullptr);
			}

			for (auto dp : _descriptorPools) {
				for (auto p : dp.second) {
					vkDestroyDescriptorPool(lDev, p.pool, nullptr);
				}
			}
		}

		/**
		 * Before drawing a frame, we may have to update some things
		 */
		virtual void vk_update(const uint32_t imageIndex, const UniformBufferType_RendererMat4& mvp) = 0;

		// /**
		//  * A renderer needs a VkSurfaceKHR to render on. The base version of vk_assignSurface enforces
		//  * this contract. However, a renderer may need additional resources, like a renderpass, swapchain
		//  * or framebuffer.
		//  */
		// virtual void vk_assignSurface(const Vk_SurfaceConfig& config) = 0;

		// /**
		//  * Remove all the resources created by vk_assignSurface.
		//  */
		// virtual void vk_removeSurface() = 0;

		bool vk_hasRenderable(const std::string& name) const {
			if (_renderables.find(name) == _renderables.end()) return false;
			return true;
		}

		bool vk_detach(const std::string& name) {
			if(_surfaceConfig.surface == nullptr){
				Vk_Logger::RuntimeError(typeid(this), "Assign a surface to the renderer before calling vk_detach()!");
			}

			if (_renderables.find(name) == _renderables.end()) {
				return false;
			}

			Vk_Logger::Log(typeid(this), GlobalCasters::castVkDetach(
				std::string("Detach object [") +
				name +
				std::string("] to Renderer [") +
				std::to_string(_surfaceConfig.viewportId) +
				std::string("]")
			));

			auto obj = _renderables.at(name);
			vk_reclaim_DescriptorSets(obj->_shaderName, obj->_sets.at(_surfaceConfig.viewportId));
			obj->_sets.erase(_surfaceConfig.viewportId);
			obj->_pipeline.erase(_surfaceConfig.viewportId);
			_renderables.erase(name);
			return true;
		}

		const bool vk_attach(std::shared_ptr<Vk_Renderable> object, bool warn=false) {
			if(_surfaceConfig.surface == nullptr){
				Vk_Logger::RuntimeError(typeid(this), "Assign a surface to the renderer before calling vk_attach()!");
			}

			if (_renderables.find(object->vk_objectName()) != _renderables.end()) {
				if(warn) Vk_Logger::Warn(typeid(this), "Object with name [{0}] is already attached to renderer!", object->vk_objectName());
				return false;
			}

			Vk_Logger::Log(typeid(this), GlobalCasters::castVkAttach(
				std::string("Attach object [") +
				object->vk_objectName() +
				std::string("] to Renderer [") +
				std::to_string(_surfaceConfig.viewportId) +
				std::string("]")
			));

			const auto& caps = _device->vk_swapchainSupportActiveDevice(_pipelineAuxilliaries.surface);
			object->_sets[_surfaceConfig.viewportId] = vk_getOrCreate_DescriptorSets(object->_shaderName, caps.nFramesInFlight);
			object->_pipeline[_surfaceConfig.viewportId] = vk_getOrCreate_GraphicsPipeline(object->_shaderName, object->_topology, object->_cullMode, object->_renderType);
			_renderables[object->vk_objectName()] = object;

			return true;
		}

		void vk_build(const Vk_Viewport& viewport, int index, const std::vector<VkCommandBuffer>& commandBuffers) {
			if(_surfaceConfig.surface == nullptr){
				Vk_Logger::RuntimeError(typeid(this), "Assign a surface to the renderer before calling vk_build()!");
			}

			Vk_Logger::Log(typeid(this), GlobalCasters::castVkBuild(
				std::string("Build Renderer[") +
				std::to_string(_surfaceConfig.viewportId) +
				std::string("]")
			));

			recordRenderingCommand(viewport, index, commandBuffers[index]);
		}

		virtual void vk_resize(const Vk_Viewport& viewport, const Vk_PipelineAuxilliaries&& auxilliaries, int index, const std::vector<VkCommandBuffer>& commandBuffers) {
			if(_surfaceConfig.surface == nullptr){
				Vk_Logger::RuntimeError(typeid(this), "Assign a surface to the renderer before calling vk_resize()!");
			}

			Vk_Logger::Log(typeid(this), GlobalCasters::castVkBuild(
				std::string("Resize Renderer[") +
				std::to_string(_surfaceConfig.viewportId) +
				std::string("]")
			));

			_pipelineAuxilliaries = std::move(auxilliaries);
			recordRenderingCommand(viewport, index, commandBuffers[index]);
		}

		Vk_Device* const vk_device() { return _device; }
		Vk_Shader* vk_getOrCreate_VertexShader(const std::string& name) {
			if (_vert.find(name) == _vert.end()) {
				Vk_Logger::Log(typeid(this), "Create Vertex Vk_Shader: [" + name + "]");
				_vert[name] = std::make_unique<Vk_Shader>(_device, _shaderPathPrefix + name + ".vert.spv");
			}
			return _vert.at(name).get();
		}

		Vk_Shader* vk_getOrCreate_FragmentShader(const std::string& name) {
			if (_frag.find(name) == _frag.end()) {
				Vk_Logger::Log(typeid(this), "Create Fragment Vk_Shader: [" + name + "]");
				_frag[name] = std::make_unique<Vk_Shader>(_device, _shaderPathPrefix + name + ".frag.spv");
			}
			return _frag.at(name).get();
		}

		I_GraphicsPipeline* vk_getOrCreate_GraphicsPipeline(
			const std::string& name, 
			Topology topology,
			CullMode cullMode,
			RenderType renderType
		) 
		{
			std::string pIdentifier = I_GraphicsPipeline::createGraphicsPipelineIndentifier(name, topology, cullMode, renderType);
			if (_pipelines.find(pIdentifier) == _pipelines.end()) {
				createGraphicsPipeline(name, pIdentifier, topology, cullMode, renderType);
			}
			return _pipelines.at(pIdentifier).get();
		}

		VkDescriptorSetLayout vk_getOrCreate_DescriptorSetLayout(const std::string& name) {
			if (_descriptorSetLayout.find(name) == _descriptorSetLayout.end()) {
				createDescriptorSetLayout(name);
			}
			return _descriptorSetLayout.at(name);
		}

		std::vector<VkDescriptorSet> vk_getOrCreate_DescriptorSets(const std::string& name, int count) {
			if (_descriptorPools.find(name) == _descriptorPools.end()) {
				createDescriptorPool(name);
			}

			std::vector<VkDescriptorSet> sets;
			auto& ds = _descriptorSets[name];
			auto& dp = _descriptorPools[name];



			auto begin = ds.begin();
			auto end = ds.end();
			if (static_cast<int>(ds.size()) >= count) {
				sets.insert(
					sets.end(),
					std::make_move_iterator(begin),
					std::make_move_iterator(begin + count)
				);
				ds.erase(begin, begin + count);

				return sets;
			}

			if (ds.size() > 0) {
				count -= static_cast<int>(ds.size());
				sets.insert(
					sets.end(),
					std::make_move_iterator(begin),
					std::make_move_iterator(end)
				);
				ds.clear();
			}

			while (count > 0) {
				// check if we have 'count' descriptor sets available
				// count should be a multiple of the number of frames in flight that we have:
				//  - double buffering => two frame buffers => two frames in flight
				// A typical call to this function will use count == number of frames in flight

				int snip = count;
				if (snip > _freshPoolSize) {
					snip = _freshPoolSize;
				}

				auto pool = &dp.back();
				if (pool->remaining - snip < 0) {
					if (pool->remaining > 0) {
						auto temp = createDescriptorSets(name, pool->pool, pool->remaining);
						sets.insert(
							sets.end(),
							std::make_move_iterator(temp.begin()),
							std::make_move_iterator(temp.end())
						);
						snip -= pool->remaining;
						pool->remaining = 0;
					}
					createDescriptorPool(name);
					pool = &dp.back();
				}

				auto temp = createDescriptorSets(name, pool->pool, snip);
				sets.insert(sets.end(), temp.begin(), temp.end());
				pool->remaining -= snip;
				count -= _freshPoolSize;
			}
			return sets;
		}

		void vk_reclaim_DescriptorSets(const std::string& name, std::vector<VkDescriptorSet>& sets) {
			// transfer the descriptors into the descriptor set list
			// get or create the appropriate list
			auto& ds = _descriptorSets[name];
			ds.insert(
				ds.end(),
				std::make_move_iterator(sets.begin()),
				std::make_move_iterator(sets.end())
			);
			sets.clear();
		}

		virtual const Vk_PipelineAuxilliaries& vk_pipelineAuxilliaries() const { 
			return _pipelineAuxilliaries; 
		}

		//Caster& Cast() {
		//	return _cast;
		//}

		// debug functions ========================================================================
		const size_t sizeofDescriptorSetBuffer(const std::string& name) {
			if (_descriptorSets.find(name) == _descriptorSets.end()) return 0;
			return _descriptorSets.at(name).size();
		}

		const size_t sizeofDescriptorPoolBuffer(const std::string& name) {
			if (_descriptorSets.find(name) == _descriptorSets.end()) return 0;
			return _descriptorPools.at(name).size();
		}

		const int sizeofLastDescriptorPool(const std::string& name) {
			if (_descriptorSets.find(name) == _descriptorSets.end()) return 0;
			return _descriptorPools.at(name).back().remaining;
		}

		std::vector<int> sizeofAllDescriptorPools(const std::string& name) {
			if (_descriptorPools.find(name) == _descriptorPools.end()) return { 0 };
			std::vector<int> res;
			auto& ds = _descriptorPools.at(name);
			std::for_each(ds.begin(), ds.end(), [&](const DescriptorPool& i) {res.push_back(i.remaining); });
			return res;
		}
		// debug functions ========================================================================

	protected:
		struct DescriptorPool {
			int remaining;
			VkDescriptorPool pool;
		};

		//Caster _cast;

		std::string _shaderPathPrefix;
		const std::string _ext_frag_spv = ".frag.spv";
		const std::string _ext_vert_spv = ".vert.spv";

		Vk_Device* const _device;
		// Vk_PipelineAuxilliaries _pipelineAuxilliaries;

		const int _freshPoolSize;
		std::map<std::string, std::shared_ptr<Vk_Renderable>> _renderables;

		std::unordered_map<std::string, std::unique_ptr<Vk_Shader>> _vert;
		std::unordered_map<std::string, std::unique_ptr<Vk_Shader>> _frag;

		std::unordered_map < std::string, VkDescriptorSetLayout> _descriptorSetLayout;
		std::unordered_map<std::string, std::vector<DescriptorPool>> _descriptorPools;
		std::unordered_map<std::string, std::vector<VkDescriptorSet>> _descriptorSets;
		std::unordered_map<std::string, std::unique_ptr<I_GraphicsPipeline>> _pipelines;

		//VkClearColorValue _clearValue;
		// view and perspective matrices for the current renderer (i.e. rasterizer etc)
		std::unique_ptr<Vk_AbstractUniformBuffer> _pv;

		Vk_SurfaceConfig _surfaceConfig;

	private:
		virtual void createGraphicsPipeline(const std::string& name, const std::string& pIdentifier, const Topology topology, const CullMode cullMode, const RenderType renderType) = 0;
		virtual void createDescriptorSetLayout(const std::string& name) = 0;
		virtual void createDescriptorPool(const std::string& name) = 0;
		virtual std::vector<VkDescriptorSet> createDescriptorSets(const std::string& name, VkDescriptorPool pool, int count) = 0;
		virtual void recordRenderingCommands(const Vk_Viewport& viewport, const std::vector<VkCommandBuffer>& commandBuffer) = 0;
		virtual void recordRenderingCommand(const Vk_Viewport& viewport, int index, VkCommandBuffer commandBuffer) = 0;
	};
}