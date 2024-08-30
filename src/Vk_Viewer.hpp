#pragma once

#include <vector>
#include <string>
#include <memory>
#include <map>
#include <atomic>
#include <fstream>
#include <functional>
#include <chrono>

#include "./external/toojpeg-master/toojpeg.hpp"

#include "Defines.h"

#include "application/Vk_Instance.hpp"
#include "Vk_ThreadPool.hpp"
#include "./camera/Vk_Camera.hpp"
#include "renderer/rasterizer/immediate_mode/Vk_Rasterizer_IM.hpp"
#include "./camera/Vk_ViewerSteering_CameraCentric.hpp"
#include "./camera/Vk_ViewerSteering_ObjectCentric.hpp"

#ifdef PYVK
	namespace py = pybind11;
#endif

namespace VK4 {
	std::ofstream vk_jpegFile;

	class Vk_Viewer {
		public:
		friend class Vk_Device;
		
		Vk_Viewer()
			:
			_surface(nullptr),
			_device(nullptr),
			_renderpass(nullptr),
			_swapchain(nullptr),
			_framebuffer(nullptr),
			_imageAvailableSemaphores({}),
			_renderFinishedSemaphores({}),
			_inFlightFences({}),
			_onResize(false),
			_rebuildBeforeNextFrame({}),
			_viewingType(Vk_ViewingType::GLOBAL),
			_lastSteeredCamera(nullptr),
			_running(false),
			_programableStop(false),
			_initWidth(0),
			_initHeight(0),
			_freshPoolSize(0),			
			_screenshotSavePath(""),
			_pause(false),
			_freshlyAttachedOrDetachedObjects(false),
			_name("Viewer")
		{}

		Vk_Viewer(
			Vk_Device* device,
			const Vk_ViewerParams& params
		) 
			: 
			_surface(nullptr),
			_device(device),
			_renderpass(nullptr),
			_swapchain(nullptr),
			_framebuffer(nullptr),
			_imageAvailableSemaphores({}),
			_renderFinishedSemaphores({}),
			_inFlightFences({}),
			_onResize(false),
			_rebuildBeforeNextFrame({}),
			_viewingType(params.viewingType),
			_lastSteeredCamera(nullptr),
			_running(false),
			_programableStop(false),
			_initWidth(params.width),
			_initHeight(params.height),
			_freshPoolSize(params.freshPoolSize),			
			_screenshotSavePath(params.screenshotSavePath),
			_pause(false),
			_freshlyAttachedOrDetachedObjects(false),
			_name(params.name)
		{
			if(!_device->vk_registerViewer(reinterpret_cast<uint64_t>(this))) {
				Vk_Logger::RuntimeError(typeid(this), "Can't use device [{0}] for multiple viewers!", _device->vk_instance()->vk_applicationName());
			}
			
			Vk_Logger::Log(typeid(this), GlobalCasters::castConstructorTitle("Create Viewer"));

			_threadPool.start(
#ifdef PYVK
				1, // no use for more on python XD
#else
				1, // until a proper mechanism can be defined, one thread is enough. It can start other threads if it likes... std::thread::hardware_concurrency(), 
#endif
				this
			);
		}

		~Vk_Viewer() {
			Vk_Logger::Log(typeid(this), GlobalCasters::castDestructorTitle("Destroy Viewer"));
			// make sure to keep the dependencies correctly (instance last, device right before that one)
			{
				auto lock = std::lock_guard<std::shared_mutex>(_runMutex);
				_programableStop = true;
			}
			_eventThread.join();
			_cameras.clear();

			auto lDev = _device->vk_lDev();

			// wait until we finished everything
			Vk_ThreadSafe::Vk_ThreadSafe_DeviceWaitIdle(lDev);

			_destroySyncResources();

			vkFreeCommandBuffers(
				_device->vk_lDev(),
				_device->vk_renderingCommandPool(),
				static_cast<uint32_t>(_commandBuffer.size()),
				_commandBuffer.data()
			);

			_framebuffer.reset();
			_swapchain.reset();
			_renderpass.reset();
			_surface.reset();
			_device->vk_unregisterViewer();
		}

		std::string vk_getVersion() {
			return "4.01";
		}

		int vk_cameraId(int x, int y){
			for(const auto& c : _cameras){
				if(c.second->vk_gridX() == x && c.second->vk_gridY() == y){
					return c.second->vk_camId();
				}
			}
			return -1;
		}

		bool vk_layoutCoords(int camId, int& x, int& y){
			for(const auto& c : _cameras){
				if(c.second->vk_camId() == camId){
					x = c.second->vk_gridX();
					y = c.second->vk_gridY();
					return true;
				}
			}
			return false;
		}

		void vk_addCamera(std::vector<Vk_CameraInit> cameras) {
			for (auto& c : cameras) {
				auto cam = std::make_unique<Vk_Camera>(c, _getSteering(c), _initWidth, _initHeight);

				_cameras.insert({ cam->vk_camId(), std::move(cam) });
			}
		}

		const Vk_CameraCoords vk_cameraCoords(int camId) const {
			if(_cameras.find(camId) == _cameras.end()){
				Vk_Logger::RuntimeError(typeid(this), "Camera with id {0} doesn't exist", camId);
			}
			return _cameras.at(camId)->vk_cameraCoords();
		}

		bool vk_attachToAll(std::shared_ptr<Vk_Renderable> object) {
			for (auto& cam : _cameras) {
				if(!vk_attachTo(cam.first, object)) {
					return false;
				}
			}
			return true;

		}

		bool vk_attachTo(int camId, std::shared_ptr<Vk_Renderable> object) {
			if(!vk_running()){
				Vk_Logger::RuntimeError(typeid(this), "Can't attach object to Viewer if it's not running. Did you forget to call vk_run() or vk_runThread()?");
			}
			if(_cameras.find(camId) == _cameras.end()){
				std::string possible = "";
				for(auto& cid : _cameras){
					possible += std::to_string(cid.first) + ",";
				}
				if(possible.ends_with(',')){
					possible = possible.substr(0, possible.size()-1);
				}
				Vk_Logger::RuntimeError(typeid(this), "Camera with id {0} does't exist. Possible are [{1}].", camId, possible);
			}

			if(_device != object->vk_device()){
				Vk_Logger::RuntimeError(
					typeid(this), 
					"Only objects with device [{0}] can be attached, not with object's [{1}] device [{2}]", 
					_device->vk_instance()->vk_applicationName(), 
					object->vk_objectName(),
					object->vk_device()->vk_instance()->vk_applicationName()
				);
				return false;
			}
			if(_cameras.at(camId)->vk_renderer()->vk_attach(object)){
				_freshlyAttachedOrDetachedObjects = true;
				// Vk_Logger::Warn(typeid(this), "Attached new object to camera {0}. Remember to call vk_rebuildAndRedraw later!", camId);
				return true;
			}
			return false;
		}

private: /* TODO: move this one */
		// this one should be private
		void vk_build() {
			Vk_ThreadSafe::Vk_ThreadSafe_DeviceWaitIdle(_device->vk_lDev());
			auto lock = AcquireGlobalLock("vk_viewer[vk_build]");
			_recordCommandBuffers();
			_freshlyAttachedOrDetachedObjects = false;
		}
public:

		void vk_rebuildAndRedraw() {
			auto lock = AcquireGlobalLock("vk_viewer[vk_rebuildAndRedraw]");
			_rebuildBeforeNextFrame = std::vector<bool>(_commandBuffer.size(), true);
			_freshlyAttachedOrDetachedObjects = false;
			vk_redraw();
		}

		void vk_detachFromAll(std::shared_ptr<Vk_Renderable> object) {
			auto objectName = object->vk_objectName();
			for (auto& cam : _cameras) {
				if(cam.second->vk_renderer()->vk_hasRenderable(objectName)){
					cam.second->vk_renderer()->vk_detach(objectName);
				}
			}
		}

		bool vk_detachFrom(int camId, std::shared_ptr<Vk_Renderable> object) {
			auto objectName = object->vk_objectName();
			if (_cameras.at(camId)->vk_renderer()->vk_hasRenderable(objectName)) {
				_cameras.at(camId)->vk_renderer()->vk_detach(objectName);
				_freshlyAttachedOrDetachedObjects = true;
				// Vk_Logger::Warn(typeid(this), "Detached old object from camera {0}. Remember to call vk_rebuildAndRedraw later if this is not the last thing you do!", camId);
				return true;
			}

			return false;
		}

		void vk_runThread() {
			_eventThread = std::thread(&Vk_Viewer::_run, this);
			while(!vk_running());
		}

		void vk_stopThread() {
			auto lock = std::lock_guard<std::shared_mutex>(_runMutex);
			_programableStop = true;
		}

		void vk_redraw() {
			{
				auto lock = std::shared_lock<std::shared_mutex>(_runMutex);
				if(_pause) return;
			}

			_surface->vk_lwws_window()->emit_windowEvent_Paint();
		}

		void vk_pauseDraw(){
			auto lock = std::lock_guard<std::shared_mutex>(_runMutex);
			_pause = true;
		}

		void vk_unpauseDraw(){
			auto lock = std::lock_guard<std::shared_mutex>(_runMutex);
			_pause = false;
		}

		// const Vk_Instance* vk_instance() const { return _instance.get(); }
		Vk_Device* const vk_device() const { 
			return _device; 
		}

		const Vk_Surface* vk_surface() const { 
			return _surface.get(); 
		}
		
		const std::unordered_map<int, std::unique_ptr<Vk_Camera>>& vk_cameras() const { 
			return _cameras; 
		}

		const Vk_Camera* vk_camera(int camId) const {
			if (_cameras.find(camId) == _cameras.end()) {
				return nullptr;
			}

			return _cameras.at(camId).get();
		}

#ifdef PYVK
		bool vk_register_action(py::object key, py::function f, int cameraId=-1){
			if(cameraId >= 0){
				Vk_Logger::RuntimeError(typeid(this), "Per camera localized actions not supported yet!");
				return false;
			}

			int intKey = tryCastKey(key);
			if(intKey < 0) {
				Vk_Logger::Error(typeid(this), "Unable to cast passed key");
				return false;
			}

			if(_actions.find(intKey) != _actions.end()){
				Vk_Logger::Error(typeid(this), "Key is already registered");
				return false;
			}
			_actions.insert({intKey, f});
			return true;
		}

		bool vk_unregister_action(py::object key){
			int intKey = tryCastKey(key);
			if(intKey < 0) {
				Vk_Logger::Error(typeid(this), "Unable to cast passed key");
				return false;
			}

			if(_actions.find(intKey) == _actions.end()){
				Vk_Logger::Error(typeid(this), "Key is not registered");
				return false;
			}

			_actions.erase(intKey);
			return true;
		}

		bool vk_exec_action(py::object key){
			int intKey = tryCastKey(key);
			if(_actions.find(intKey) == _actions.end()){
				Vk_Logger::Error(typeid(this), "Key is not registered");
				return false;
			}

			vk_execAction(intKey);
			return true;
		}
#endif
		template<class ObjType>
		bool vk_registerAction(LWWS::LWWS_Key::Special key, ObjType* obj, t_func<ObjType> f, int cameraId=-1){
			return vk_registerAction(LWWS::LWWS_Key::KeyToInt(key), obj, f, cameraId);
		}

		template<class ObjType>
		bool vk_registerAction(char c, ObjType* obj, t_func<ObjType> f, int cameraId=-1){
			return vk_registerAction(LWWS::LWWS_Key::KeyToInt(c), obj, f, cameraId);
		}

		template<class ObjType>
		bool vk_registerAction(int key, ObjType* obj, t_func<ObjType> f, int cameraId=-1){
			if(cameraId >= 0){
				Vk_Logger::Warn(typeid(this), "Per camera localized actions not supported yet!");
				return false;
			}
			else{
				if(_actions.find(key) != _actions.end()){
					Vk_Logger::Warn(typeid(this), "Tried to register the same key twice! Unregister key first.");
					return false;
				}
				_actions.insert({
					key,
					Vk_TFunc(obj, f, {}).get()
				});
				return true;
			}
		}

		bool vk_unregisterAction(LWWS::LWWS_Key::Special key){
			return vk_unregisterAction(LWWS::LWWS_Key::KeyToInt(key));
		}

		bool vk_unregisterAction(char key){
			return vk_unregisterAction(LWWS::LWWS_Key::KeyToInt(key));
		}

		bool vk_unregisterAction(int key){
			if(_actions.find(key) == _actions.end()){
				Vk_Logger::Warn(typeid(this), "Tried to unregister non-existing key! Register key first.");
				return false;
			}

			_actions.erase(key);
			return true;
		}

		void vk_execAction(LWWS::LWWS_Key::Special key){
			vk_execAction(LWWS::LWWS_Key::KeyToInt(key));
		}

		void vk_execAction(char key){
			vk_execAction(LWWS::LWWS_Key::KeyToInt(key));
		}

		void vk_execAction(int key){
			if(_actions.find(key) != _actions.end()){
#ifdef PYVK
				_threadPool.enqueueJob(&_actions.at(key), std::bind(&Vk_Viewer::_redraw, this));
#else
				_threadPool.enqueueJob(_actions.at(key), std::bind(&Vk_Viewer::_redraw, this));
#endif
			}
		}

		bool vk_running(){
			auto lock = std::shared_lock<std::shared_mutex>(_runMutex);
			return _running;
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
		// std::unique_ptr<Vk_Instance> _instance;
		std::unique_ptr<Vk_Surface> _surface;

		Vk_Device* _device;

		std::unique_ptr<I_RenderPass> _renderpass;
		std::unique_ptr<I_Swapchain> _swapchain;
		std::unique_ptr<I_FrameBuffer> _framebuffer;

		// synchronization parts
		// A semaphore is used to add order between queue operations.
		std::vector<VkSemaphore> _imageAvailableSemaphores;
		std::vector<VkSemaphore> _renderFinishedSemaphores;
		std::vector<VkFence> _inFlightFences;
		
		bool _onResize;
		std::vector<bool> _rebuildBeforeNextFrame;

		// individual or collective steering
		Vk_ViewingType _viewingType;
		Vk_Camera* _lastSteeredCamera;

		// add a threadpool
		Vk_ThreadPool _threadPool;
		bool _running = false;
		bool _programableStop = false;
		std::thread _eventThread;
		std::shared_mutex _runMutex;
		int _initWidth;
		int _initHeight;
#ifdef PYVK
		std::map<int, py::function> _actions;
#else
		std::map<int, std::shared_ptr<VK4::Vk_Func>> _actions;
#endif

		int _freshPoolSize;
		std::unordered_map<int, std::unique_ptr<Vk_Camera>> _cameras;
		std::vector<VkCommandBuffer> _commandBuffer;

		std::string _screenshotSavePath;
		bool _pause;
		bool _freshlyAttachedOrDetachedObjects;

		std::string _name;


// ############################################################################################################
//                █████   █████  ██████  ███████ ███████ █     █  █████  █     █ ███████ ███████               
//               █     █ █     █ █     █ █       █       ██    █ █     █ █     █ █     █    █                  
//               █       █       █     █ █       █       █ █   █ █       █     █ █     █    █                  
//                █████  █       ██████  █████   █████   █  █  █  █████  ███████ █     █    █                  
//                     █ █       █   █   █       █       █   █ █       █ █     █ █     █    █                  
//               █     █ █     █ █    █  █       █       █    ██ █     █ █     █ █     █    █                  
//                █████   █████  █     █ ███████ ███████ █     █  █████  █     █ ███████    █                  
// ############################################################################################################
		static void _output(unsigned char byte) {
			vk_jpegFile << byte;
		}

		void _imageToJpegImage(std::string filename, const Screenshot& screenshot) {
			if(_screenshotSavePath.back() != '/') _screenshotSavePath += "/";
			std::string fname = _screenshotSavePath + filename;
			vk_jpegFile = std::ofstream(fname, std::ios_base::out | std::ios_base::binary);

			size_t s = screenshot.color.size();
			std::vector<uint8_t> vec;
			vec.reserve(screenshot.width * screenshot.height * 3);
			for (size_t i = 0; i < s; i += 4) {
				uint8_t b = screenshot.color[i];
				uint8_t g = screenshot.color[i + 1];
				uint8_t r = screenshot.color[i + 2];
				// uint8_t a = screenshot.color[i + 3];

				vec.push_back(r);
				vec.push_back(g);
				vec.push_back(b);
			}

			// const int bytesPerPixel = 3;
			const bool isRGB = true;
			const int quality = 90;
			const bool downsample = false;
			const char* comment = fname.c_str();
			TooJpeg::writeJpeg(_output, vec.data(), screenshot.width, screenshot.height, isRGB, quality, downsample, comment);
			vk_jpegFile.close();
		}

		void _screenshot(std::string filename)
		{
			const auto screenshot = _swapchain->vk_getFrameBuffer(_device->bridge.currentFrame());
			_imageToJpegImage(filename, screenshot);
		}

// ############################################################################################################
//         ██████  ██████  ███ █     █         █     █ ███████ ███████ █     █ ███████ ██████   █████          
//         █     █ █     █  █  █     █         ██   ██ █          █    █     █ █     █ █     █ █     █         
//         █     █ █     █  █  █     █         █ █ █ █ █          █    █     █ █     █ █     █ █               
//         ██████  ██████   █  █     █         █  █  █ █████      █    ███████ █     █ █     █  █████          
//         █       █   █    █   █   █          █     █ █          █    █     █ █     █ █     █       █         
//         █       █    █   █    █ █           █     █ █          █    █     █ █     █ █     █ █     █         
//         █       █     █ ███    █            █     █ ███████    █    █     █ ███████ ██████   █████          
// ############################################################################################################
		void _run() {
			_surface = std::make_unique<Vk_Surface>(_device->vk_instance(), _name, _initWidth, _initHeight, true, true);
			_surface->vk_lwws_window()->bind_IntKey_Callback(this, &Vk_Viewer::_onKey);
			_surface->vk_lwws_window()->bind_MouseAction_Callback(this, &Vk_Viewer::_onMouseAction);
			_surface->vk_lwws_window()->bind_WindowState_Callback(this, &Vk_Viewer::_onWindowAction);

			auto window = _surface->vk_lwws_window();
			window->windowEvents_Init();

			_renderpass = std::make_unique<Vk_RenderPass_IM>(_device, _surface.get());
			_swapchain = std::make_unique<Vk_Swapchain_IM>(_device, _surface.get());
			_framebuffer = std::make_unique<Vk_Framebuffer_IM>(_device, _surface.get(), _swapchain.get(), _renderpass.get());

			const auto& caps = _device->vk_swapchainSupportActiveDevice(_surface.get());
			_commandBuffer.resize(caps.nFramesInFlight);
			VkCommandBufferAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			allocInfo.commandPool = _device->vk_renderingCommandPool();
			allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			allocInfo.commandBufferCount = static_cast<uint32_t>(_commandBuffer.size());

			Vk_CheckVkResult(typeid(this), vkAllocateCommandBuffers(
				_device->vk_lDev(),
				&allocInfo,
				_commandBuffer.data()),
				"Failed to allocate command buffers!"
			);

			_createSyncResources();

			for(auto& cam : _cameras){
				_attachRenderer(
					cam.second.get(), 
					I_Renderer::Vk_PipelineAuxilliaries {
						.surface=_surface.get(), 
						.renderpass=_renderpass.get(),
						.swapchain=_swapchain.get(),
						.framebuffer=_framebuffer.get()
					}
				);
			}

			vk_build();
			window->emit_windowEvent_Paint();

			{
				auto lock = std::lock_guard<std::shared_mutex>(_runMutex);
				_running = true;
			}
			_onResize = false;

			while(window->windowEvents_Exist()){
				// _device->vk_cleanSingleTimeCommands();
				window->windowEvents_Pump();

				{
					auto lock = std::shared_lock<std::shared_mutex>(_runMutex);
					if(_programableStop) goto terminate;
				}

				if(window->windowShouldClose()){
					goto terminate;
				}
			}

			terminate:
			Vk_ThreadSafe::Vk_ThreadSafe_DeviceWaitIdle(_device->vk_lDev());
			{
				auto lock = std::lock_guard<std::shared_mutex>(_runMutex);
				_running = false;
				_programableStop = false;
			}
			_threadPool.stop();
		}

		std::unique_ptr<I_ViewerSteering> _getSteering(const Vk_CameraInit& init){
			if(init.specs.steeringType == Vk_SteeringType::CAMERA_CENTRIC){
				return std::make_unique<Vk_ViewerSteering_CameraCentric>();
			}
			else if(init.specs.steeringType == Vk_SteeringType::OBJECT_CENTRIC){
				return std::make_unique<Vk_ViewerSteering_ObjectCentric>();
			}
			else {
				return std::make_unique<Vk_ViewerSteering_CameraCentric>();
			}
		}

		void _recordCommandBuffers(bool resize=false) {
			// auto lock = AcquireGlobalLock("vk_viewer[_recordCommandBuffers]");
			_rebuildBeforeNextFrame = std::vector<bool>(_commandBuffer.size(), false);
			for(size_t i=0; i<_commandBuffer.size(); ++i){
				_recordCommandBuffer(static_cast<int>(i), resize);
			}
		}

		void _recordCommandBuffer(int index, bool resize=false) {

			VkExtent2D extent = _device->vk_swapchainSupportActiveDevice(_surface.get()).capabilities.currentExtent;
			auto& scFrameBuffers = *_framebuffer->vk_frameBuffers();
			_rebuildBeforeNextFrame.at(index) = false;

			VkClearColorValue _clearValue;
			_clearValue.float32[0] = 0.1f;
			_clearValue.float32[1] = 0.1f;
			_clearValue.float32[2] = 0.1f;
			_clearValue.float32[3] = 1.0f;

			VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

			Vk_CheckVkResult(typeid(this), vkBeginCommandBuffer(_commandBuffer[index], &beginInfo), "Failed to begin recording command buffer!");

			VkRenderPassBeginInfo renderPassInfo{};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = _renderpass->vk_renderPass();
			renderPassInfo.framebuffer = scFrameBuffers[index];

			std::array<VkClearValue, 2> clearValues{};
			clearValues[0].depthStencil = { 1.0f, 0 };
			clearValues[1].color = _clearValue;
			renderPassInfo.clearValueCount = 2;
			renderPassInfo.pClearValues = clearValues.data();
			renderPassInfo.renderArea.offset = { 0,0 };
			renderPassInfo.renderArea.extent = extent;

			vkCmdBeginRenderPass(_commandBuffer[index], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

			if (!resize) {
				for (auto& c : _cameras) {
					c.second->vk_renderer()->vk_build(c.second->vk_viewport(), index, _commandBuffer);
				}
			}
			else {
				auto originalExtent = _surface->vk_canvasOriginalSize();
				for (auto& c : _cameras) {
					float ow = static_cast<float>(originalExtent.width);
					float oh = static_cast<float>(originalExtent.height);

					float x = static_cast<float>(c.second->vk_originalX()) / ow;
					float y = static_cast<float>(c.second->vk_originalY()) / oh;

					float w = static_cast<float>(c.second->vk_originalWidth()) / ow;
					float h = static_cast<float>(c.second->vk_originalHeight()) / oh;

					c.second->vk_viewport(
						Vk_Viewport {
							.x=static_cast<int32_t>(std::roundf(x * extent.width)),
							.y=static_cast<int32_t>(std::roundf(y * extent.height)),
							.width=static_cast<uint32_t>(std::roundf(w * extent.width)),
							.height=static_cast<uint32_t>(std::roundf(h * extent.height))
						}
					);

					c.second->vk_calculateTransform();
					
					c.second->vk_renderer()->vk_resize(
						c.second->vk_viewport(),
						I_Renderer::Vk_PipelineAuxilliaries{
									.surface = _surface.get(),
									.renderpass = _renderpass.get(),
									.swapchain = _swapchain.get(),
									.framebuffer = _framebuffer.get()
								},
						index,
						_commandBuffer
					);
				}
			}

			vkCmdEndRenderPass(_commandBuffer[index]);
			Vk_CheckVkResult(typeid(this), vkEndCommandBuffer(_commandBuffer[index]), "Failed to record command buffer!");
		}

		void _redraw() {
			// glfwPostEmptyEvent();
		}

		VkResult _drawFrame() {
			auto lDev = _device->vk_lDev();
			int nFramesInFlight = static_cast<int>(_renderFinishedSemaphores.size()); // doesn't matter which one...

			{
				auto lock = AcquireGlobalLock("vk_viewer[_drawFrame]");
				int currentFrame = _device->bridge.currentFrame();

				// We have to wait for the current fence to be signaled to make sure that
				VkResult res = vkWaitForFences(lDev, 1, &_inFlightFences[currentFrame], VK_TRUE, GLOBAL_FENCE_TIMEOUT);
				if(res == VK_TIMEOUT){
					Vk_Logger::RuntimeError(typeid(this), "drawFrame timeout for frame index [{0}]", currentFrame);
					return res;
				}
				else if (res != VK_SUCCESS) {
					Vk_Logger::RuntimeError(typeid(this), "Waiting for fences had catastrphic result!");
				}

				if(_rebuildBeforeNextFrame.at(currentFrame)){
					_device->bridge.runCurrentFrameUpdates();
					_recordCommandBuffer(currentFrame);
				}

				// acquire the next imageenqueueJob
				uint32_t imageIndex;
				VkResult result = vkAcquireNextImageKHR(
					lDev, _swapchain->vk_swapchain(),
					GLOBAL_FENCE_TIMEOUT,
					_imageAvailableSemaphores[currentFrame], // this one is signaled once the presentation queue is done with this image
					VK_NULL_HANDLE, &imageIndex
				);

				// catch some exceptions
				if(result != VK_SUCCESS){
					switch(result){
						case VK_ERROR_OUT_OF_DATE_KHR:
							Vk_Logger::Log(typeid(this), "vkAcquireNextImageKHR result out of date, reset viewer [VK_SUBOPTIMAL_KHR]!");
							_resetViewer();
							return result;
						case VK_SUBOPTIMAL_KHR:
							Vk_Logger::Log(typeid(this), "vkAcquireNextImageKHR result suboptimal, reset viewer [VK_SUBOPTIMAL_KHR]!");
							_resetViewer();
							return result;
						default:
							Vk_Logger::RuntimeError(typeid(this), "vkAcquireNextImageKHR unknown error [{0}]!", static_cast<int>(result));
					}
				}

				// otherwise update the uniform buffer with the mvp matrix
				// Note: We always have this buffer. It belongs to the renderer. 
				//       The only thing that matters is if it's used in the current shader or not.
				//       the binding point for the uniform buffer is set in the 'createDescriptorSetLayout' 
				//       for every individual shader. Here we just have to update the data.

				for (auto& c : _cameras) {
					auto& cam = c.second;
					cam->vk_update(currentFrame);
				}

				// reset the fence we waited for because we updated all the camera shaders with the
				// potentially new view perspective => GPU can go on...
				vkResetFences(lDev, 1, &_inFlightFences[currentFrame]);

				VkSubmitInfo submitInfo{};
				submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

				VkSemaphore waitSemaphores[] = { _imageAvailableSemaphores[currentFrame] };
				VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
				submitInfo.waitSemaphoreCount = 1;
				submitInfo.pWaitSemaphores = waitSemaphores;
				submitInfo.pWaitDstStageMask = waitStages;
				submitInfo.commandBufferCount = 1;
				submitInfo.pCommandBuffers = &_commandBuffer[currentFrame];

				VkSemaphore signalSemaphores[] = { _renderFinishedSemaphores[currentFrame] };
				submitInfo.signalSemaphoreCount = 1;
				submitInfo.pSignalSemaphores = signalSemaphores;
				
				{
					Vk_CheckVkResult(typeid(this), 
						Vk_ThreadSafe::Vk_ThreadSafe_QueueSubmit(
							_device->vk_graphicsQueue(),
							1,
							&submitInfo,
							_inFlightFences[currentFrame]
						),
						"Failed to submit draw command buffer!"
					);			
				}

				VkPresentInfoKHR presentInfo{};
				presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
				presentInfo.waitSemaphoreCount = 1;
				presentInfo.pWaitSemaphores = signalSemaphores;

				VkSwapchainKHR swapChains[] = { _swapchain->vk_swapchain() };
				presentInfo.swapchainCount = 1;
				presentInfo.pSwapchains = swapChains;
				presentInfo.pImageIndices = &imageIndex;

				result = vkQueuePresentKHR(
					_device->vk_presentationQueue(),
					&presentInfo
				);

				if(result != VK_SUCCESS){
					switch(result) {
						case VK_SUBOPTIMAL_KHR:
							Vk_Logger::Log(typeid(this), "vkQueuePresentKHR subission suboptimal, reset viewer [VK_SUBOPTIMAL_KHR]!");
							_resetViewer();
							return result;
						break;
						case VK_ERROR_OUT_OF_DATE_KHR:
							Vk_Logger::Log(typeid(this), "vkQueuePresentKHR subission out of date, reset viewer [VK_ERROR_OUT_OF_DATE_KHR]!");
							_resetViewer();
							return result;
						break;
							case VK_ERROR_OUT_OF_HOST_MEMORY:
							Vk_Logger::RuntimeError(typeid(this), "vkQueuePresentKHR subission failed [VK_ERROR_OUT_OF_HOST_MEMORY]!");
						break;
							case VK_ERROR_OUT_OF_DEVICE_MEMORY:
							Vk_Logger::RuntimeError(typeid(this), "vkQueuePresentKHR subission failed [VK_ERROR_OUT_OF_DEVICE_MEMORY]!");
						break;
						case VK_ERROR_DEVICE_LOST:
							Vk_Logger::RuntimeError(typeid(this), "vkQueuePresentKHR subission failed [VK_ERROR_DEVICE_LOST]!");
							break;
						case VK_ERROR_SURFACE_LOST_KHR:
							Vk_Logger::RuntimeError(typeid(this), "vkQueuePresentKHR subission failed [VK_ERROR_SURFACE_LOST_KHR]!");
							break;
						case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT:
							Vk_Logger::RuntimeError(typeid(this), "vkQueuePresentKHR subission failed [VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT]!");
							break;
						default:
							Vk_Logger::RuntimeError(typeid(this), "vkQueuePresentKHR submission unknown error [{0}]", static_cast<int>(result));
					}
				}

				_device->bridge.incrFrameNr();
			}

			return VK_SUCCESS;
		}

		inline void _attachRenderer(Vk_Camera* cam, I_Renderer::Vk_PipelineAuxilliaries auxilliaries) {
			switch (cam->vk_type()) {
			case Vk_CameraType::Rasterizer_IM:
				cam->vk_renderer(std::make_unique<Vk_Rasterizer_IM>(
					cam->vk_camId(),
					_device,
					auxilliaries,
					_freshPoolSize,
					VK4::UniformBufferType_RendererMat4{ .mat = cam->vk_perspective() * cam->vk_view() }
				));
				break;
			default:
				throw UnknownCameraTypeException();
			}
		}

		void _destroySyncResources() {
			VkDevice lDev = _device->vk_lDev();
			const auto& caps = _device->vk_swapchainSupportActiveDevice(_surface.get());
			int nFramesInFlight = caps.nFramesInFlight;
			
			for (int i = 0; i < nFramesInFlight; i++) {
				vkDestroySemaphore(lDev, _renderFinishedSemaphores[i], nullptr);
				vkDestroySemaphore(lDev, _imageAvailableSemaphores[i], nullptr);
				vkDestroyFence(lDev, _inFlightFences[i], nullptr);
			}

			_imageAvailableSemaphores.clear();
			_renderFinishedSemaphores.clear();
			_inFlightFences.clear();
		}

		void _createSyncResources() {
			// create synchronization resources
			VkDevice lDev = _device->vk_lDev();
			const auto& caps = _device->vk_swapchainSupportActiveDevice(_surface.get());
			int nFramesInFlight = caps.nFramesInFlight;

			_imageAvailableSemaphores.resize(nFramesInFlight);
			_renderFinishedSemaphores.resize(nFramesInFlight);
			_inFlightFences.resize(nFramesInFlight);

			VkSemaphoreCreateInfo semaphoreInfo{};
			semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

			VkFenceCreateInfo fenceInfo{};
			fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
			// we must create the fences in the signaled state to ensure that on the first call to drawFrame
			// vkWaitForFences won't wait indefinitely
			fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

			for (int i = 0; i < nFramesInFlight; i++) {
				Vk_CheckVkResult(typeid(this), vkCreateSemaphore(lDev, &semaphoreInfo, nullptr, &_imageAvailableSemaphores[i]), "Failed to create image available semaphore for a frame!");
				Vk_CheckVkResult(typeid(this), vkCreateSemaphore(lDev, &semaphoreInfo, nullptr, &_renderFinishedSemaphores[i]), "Failed to create render finished semaphore");
				Vk_CheckVkResult(typeid(this), vkCreateFence(lDev, &fenceInfo, nullptr, &_inFlightFences[i]), "Failed to create in flight fences");
			}
		}

		void _resetViewer() {
			_onResize = true;
			Vk_ThreadSafe::Vk_ThreadSafe_DeviceWaitIdle(_device->vk_lDev());

			_destroySyncResources(); // must be done before swapchainSupport is invalidated to keep correct nFramesInFlight!
			_device->vk_invalidateSwapchainSupport();
			_device->vk_swapchainSupportActiveDevice(_surface.get());
			_renderpass.reset();
			_swapchain.reset();
			_framebuffer.reset();

			_renderpass = std::make_unique<Vk_RenderPass_IM>(_device, _surface.get());
			_swapchain = std::make_unique<Vk_Swapchain_IM>(_device, _surface.get());
			_framebuffer = std::make_unique<Vk_Framebuffer_IM>(_device, _surface.get(), _swapchain.get(), _renderpass.get());
			_createSyncResources(); // must be done after swapchain creation to get correct nFramesInFlight

			_recordCommandBuffers(true);
			_device->bridge.setCurrentFrameTo(0);
			_onResize = false;
		}


// ############################################################################################################
//                     █████     █    █       █       ██████     █     █████  █    █  █████                    
//                    █     █   █ █   █       █       █     █   █ █   █     █ █   █  █     █                   
//                    █        █   █  █       █       █     █  █   █  █       █  █   █                         
//                    █       █     █ █       █       ██████  █     █ █       ███     █████                    
//                    █       ███████ █       █       █     █ ███████ █       █  █         █                   
//                    █     █ █     █ █       █       █     █ █     █ █     █ █   █  █     █                   
//                     █████  █     █ ███████ ███████ ██████  █     █  █████  █    █  █████                    
// ############################################################################################################
		void _onWindowAction(int w, int h, int px, int py, const std::set<int>& pressedKeys, LWWS::WindowAction windowAction, void* aptr){
			if(windowAction == LWWS::WindowAction::Resized){
				auto lock = AcquireGlobalLock("vk_viewer[_onWindowAction(resetViewer)]");
				_resetViewer();
			}
			else if(windowAction == LWWS::WindowAction::Paint){
				_drawFrame();
			}
		}

		void _onMouseAction(int px, int py, int dx, int dy, float dz, const std::set<int>& pressedKeys, LWWS::MouseButton mouseButton, LWWS::ButtonOp op, LWWS::MouseAction mouseAction, void* aptr){
			// std::cout << px << " " << py << " " << dx << " " << dy << " " << dz << " " << LWWS::LWWS_Key::IntKey2String(pressedKeys) << " " << LWWS::MouseButton2String(mouseButton) << " " << LWWS::ButtonOp2String(op) << " " << LWWS::MouseAction2String(mouseAction) << std::endl;
			if(mouseAction == LWWS::MouseAction::MouseScroll){
				auto lctrl = LWWS::LWWS_Key::KeyToInt(LWWS::LWWS_Key::Special::LControl);
				auto rctrl = LWWS::LWWS_Key::KeyToInt(LWWS::LWWS_Key::Special::RControl);
				auto lshift = LWWS::LWWS_Key::KeyToInt(LWWS::LWWS_Key::Special::LShift);
				auto rshift = LWWS::LWWS_Key::KeyToInt(LWWS::LWWS_Key::Special::RShift);

				bool ctrlPressed = pressedKeys.find(lctrl) != pressedKeys.end() || pressedKeys.find(rctrl) != pressedKeys.end();
				bool shiftPressed = pressedKeys.find(lshift) != pressedKeys.end() || pressedKeys.find(rshift) != pressedKeys.end();

				if(!ctrlPressed && !shiftPressed){
					dz *= 0.05f;
				}
				else if(!ctrlPressed){
					dz *= 0.20f;
				}
				else if(!shiftPressed){
					dz *= 0.5f;
				}

				const auto& cameras = vk_cameras();
				if(_viewingType == Vk_ViewingType::GLOBAL){
					for(const auto& c : cameras){
						c.second->onMouseAction(px, py, dx, dy, dz, pressedKeys, mouseButton, op, mouseAction, aptr);
					}
					vk_redraw();
				}
				else {
					for(const auto& c : cameras){
						if(c.second->vk_contains(c.second->vk_lastMousePosX(), c.second->vk_lastMousePosY())){
							c.second->onMouseAction(px, py, dx, dy, dz, pressedKeys, mouseButton, op, mouseAction, aptr);
							vk_redraw();
							break;
						}
					}
				}
			}
			else if(mouseAction == LWWS::MouseAction::MouseButton && mouseButton == LWWS::MouseButton::Left && op == LWWS::ButtonOp::Down){
				const auto& cameras = vk_cameras();
				if(_viewingType == Vk_ViewingType::LOCAL){
					for(const auto& c : cameras){
						if(c.second->vk_contains(px, py)){
							_lastSteeredCamera = c.second.get();
							break;
						}
					}
				}
			}
			else if(mouseAction == LWWS::MouseAction::MouseMove && mouseButton == LWWS::MouseButton::Left && op == LWWS::ButtonOp::SteadyPress){
				const auto& cameras = vk_cameras();
				if(_viewingType == Vk_ViewingType::GLOBAL){
					for(const auto& c : cameras){
						c.second->onMouseAction(px, py, dx, dy, dz, pressedKeys, mouseButton, op, mouseAction, aptr);
					}
					vk_redraw();
				}
				else {
					_lastSteeredCamera->onMouseAction(px, py, dx, dy, dz, pressedKeys, mouseButton, op, mouseAction, aptr);
					vk_redraw();
				}
			}
			else if(mouseAction == LWWS::MouseAction::MouseButton && mouseButton == LWWS::MouseButton::Left && op == LWWS::ButtonOp::Up){
				_lastSteeredCamera = nullptr;
			}

			for(const auto& c : vk_cameras()){
				c.second->vk_lastMousePosition(px, py);
			}
		}

		void _onKey(int k, LWWS::ButtonOp op, const std::set<int>& otherPressedKeys, void* aptr){
			int lctrl = LWWS::LWWS_Key::KeyToInt(LWWS::LWWS_Key::Special::LControl);
			int rctrl = LWWS::LWWS_Key::KeyToInt(LWWS::LWWS_Key::Special::RControl);
			if (op == LWWS::ButtonOp::Down) {
				// int lshift = LWWS::LWWS_Key::KeyToInt(LWWS::LWWS_Key::Special::LShift);
				// int rshift = LWWS::LWWS_Key::KeyToInt(LWWS::LWWS_Key::Special::RShift);

				bool ctrlPressed = otherPressedKeys.contains(lctrl) || otherPressedKeys.contains(rctrl);
				int sKey = LWWS::LWWS_Key::KeyToInt('s');
				bool sPressed = otherPressedKeys.contains(sKey);

				if (ctrlPressed && sPressed) {
					long long timestamp = std::chrono::high_resolution_clock::now().time_since_epoch().count();
					std::string filename = std::string("screenshot_") + std::to_string(timestamp) + std::string(".jpeg");
					Vk_Logger::Message(typeid(NoneObj), "Take screenshot: {0}", filename);
					_screenshot(filename);
				}

				// todo: do some translating
				vk_execAction(k);
			}
		}
	};
}