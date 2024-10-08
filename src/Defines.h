#pragma once
#define PYVK
// some generic way to distinguish operating systems
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
   //define something for Windows (32-bit and 64-bit, this part is common)
   #ifdef _WIN64
      //define something for Windows (64-bit only)
	  #define PLATFORM_WINDOWS_x64
   #else
      //define something for Windows (32-bit only)
	  #define PLATFORM_WINDOWS_x32
   #endif
   #define VK_USE_PLATFORM_WIN32_KHR
#elif __APPLE__
    #include <TargetConditionals.h>
    #if TARGET_IPHONE_SIMULATOR
         // iOS, tvOS, or watchOS Simulator
    #elif TARGET_OS_MACCATALYST
         // Mac's Catalyst (ports iOS API into Mac, like UIKit).
    #elif TARGET_OS_IPHONE
        // iOS, tvOS, or watchOS device
    #elif TARGET_OS_MAC
        // Other kinds of Apple platforms
		#define PLATFORM_MAC
    #else
    #   error "Unknown Apple platform"
    #endif
#elif __ANDROID__
    // Below __linux__ check should be enough to handle Android,
    // but something may be unique to Android.
#elif __linux__
    // linux
	#define PLATFORM_LINUX
	// #define VK_USE_PLATFORM_XCB_KHR
	#define VK_USE_PLATFORM_XLIB_KHR
#elif __unix__ // all unices not caught above
    // Unix
#elif defined(_POSIX_VERSION)
    // POSIX
#else
#   error "Unknown compiler"
#endif

#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define NOMINMAX

#include <vulkan/vulkan.h>
#include <mutex>
#include <iostream>
#include <functional> //for std::hash
#include <string>
#include <memory>
#include <streambuf>
#include <array>
#include <thread>

#pragma warning(push)
#pragma warning(disable : 4196)
#include "glm/glm.hpp" // vectors and matrices
#include "glm/gtc/matrix_transform.hpp"
#include "glm/ext.hpp"
#include "glm/gtx/string_cast.hpp"
#pragma warning(pop)

#undef max
#undef min

#ifdef PYVK
	#include <pybind11/pybind11.h>
	#include <pybind11/numpy.h>
	#include <pybind11/stl.h>
	#include <pybind11/functional.h>
	#include <pybind11/embed.h>

	namespace py = pybind11;
#endif

#include <cinttypes>
#include "Vk_Coloring.hpp"

#define PRECISION 1e-6
#define TYPE_VULKAN 'V'
#define TYPE_SOFTWARE 'S'
#define TYPE_DUMMY 'D'

#include "./lwws_win/lwws_win.hpp"
#include "Vk_Logger.hpp"

// #define THREAD_DEBUGGING_GLOBAL

namespace VK4 {
	//static VkSampleCountFlagBits offscreen_samples = VK_SAMPLE_COUNT_1_BIT;

	typedef float point_type;
	typedef std::uint32_t index_type;

	// template<class ObjType>
	// typedef void(ObjType::* mf_type)(_Types...);

	template<typename ObjType>
	using t_func = void (ObjType::*)(std::function<void()>);

	std::recursive_mutex global_mutex;
	std::mutex queue_submit_mutex;
	std::mutex queue_wait_idle_mutex;
	std::mutex device_wait_idle_mutex;

#ifdef THREAD_DEBUGGING_GLOBAL
	std::string who_has_global_mutex_info;
#endif
// #ifdef THREAD_DEBUGGING_QUEUE_SUBMIT
// 	std::string who_has_queue_submit_mutex_info;
// #endif
	// static std::hash<std::string> global_hasher;

	constexpr uint64_t GLOBAL_FENCE_TIMEOUT = static_cast<uint64_t>(100e6); // in nanosecons => 100e6 = 100ms

	constexpr int MINIMAL_WIDTH = 512;
	constexpr int MINIMAL_HEIGHT = 512;

// 	class AcquireQueueSubmitLock {
// 	public:
// 		AcquireQueueSubmitLock(const char* info) {
// #ifdef THREAD_DEBUGGING_QUEUE_SUBMIT
// 			std::cout << "[AcquireQueueSubmitLock] ===> waiting: " << info << " === for ===> " << who_has_queue_submit_mutex_info << std::endl;
// #endif
// 			queue_submit_mutex.lock();
// #ifdef THREAD_DEBUGGING_QUEUE_SUBMIT
// 			who_has_queue_submit_mutex_info = std::string(info);
// 			std::cout << who_has_queue_submit_mutex_info << std::endl;
// #endif
// 		}
// 		~AcquireQueueSubmitLock() {
// 			queue_submit_mutex.unlock();
// #ifdef THREAD_DEBUGGING_QUEUE_SUBMIT
// 			std::cout << "[AcquireQueueSubmitLock] " << who_has_queue_submit_mutex_info << " ===> released" << std::endl;
// 			who_has_queue_submit_mutex_info = std::string("no one");
// #endif
// 		}
// 	};

	class AcquireGlobalLock {
	public:
		AcquireGlobalLock(const char* info) {
#ifdef THREAD_DEBUGGING_GLOBAL
			std::cout << "[AcquireGlobalLock] ===> waiting: " << info << " === for ===> " << who_has_global_mutex_info << std::endl;
#endif
			global_mutex.lock();
#ifdef THREAD_DEBUGGING_GLOBAL
			who_has_global_mutex_info = std::string(info);
			std::cout << who_has_global_mutex_info << std::endl;
#endif
		}
		~AcquireGlobalLock() {
			global_mutex.unlock();
#ifdef THREAD_DEBUGGING_GLOBAL
			std::cout << "[AcquireGlobalLock] " << who_has_global_mutex_info << " ===> released" << std::endl;
			who_has_global_mutex_info = std::string("no one");
#endif
		}
	};

	class Vk_ThreadSafe {
	public:
		static VkResult Vk_ThreadSafe_QueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence){
			// std::cout << "wait for queue submit" << std::endl;
			auto lock = std::lock_guard<std::mutex>(queue_submit_mutex);
			// std::cout << "got queue submit" << std::endl;
			return vkQueueSubmit(queue, submitCount, pSubmits, fence);
		}

		static VkResult Vk_ThreadSafe_QueueWaitIdle(VkQueue queue){
			auto lock = std::lock_guard<std::mutex>(queue_wait_idle_mutex);
			return vkQueueWaitIdle(queue);
		}

		static VkResult Vk_ThreadSafe_DeviceWaitIdle(VkDevice device){
			auto lock = std::lock_guard<std::mutex>(device_wait_idle_mutex);
			return vkDeviceWaitIdle(device);
		}
	};

#ifdef PYVK
    class Vk_NumpyTransformers {
    public:
        template<class T>
        static glm::tmat4x4<T> arrayToGLM4x4(const py::array_t<point_type, py::array::c_style>& inData){
            py::buffer_info pInfo = inData.request();
            T* p = static_cast<T*>(pInfo.ptr);
            return glm::make_mat4x4(p);
        }

        template<class T>
        static glm::tvec3<T> arrayToGLMv3(const py::array_t<T, py::array::c_style>& arr){
			py::buffer_info info = arr.request();
			T* ptr = static_cast<T*>(info.ptr);
			return glm::make_vec3(ptr);
        }

        template<class T>
        static T* structArrayToCpp(const py::array_t<point_type, py::array::c_style>& inData, size_t& outLen){
            // NOTE: this way of passing numpy data is absolutely not copying anything
            // For example, the following code
            // 		std::cout << glm::to_string(p[0].pos) << std::endl;
            //		p[0].pos.x = 5.5f;
            // 		std::cout << glm::to_string(p[0].pos) << std::endl;
            // will output 5.5 as the x-component of the first entry
            // If we then output the first entry of the numpy array on the python side,
            // we get the same thing

            // using 
            // 		py::array_t<VK4::point_type, py::array::c_style>& points
            // or
            // 		py::array_t<VK4::point_type, py::array::c_style> points
            // doesn't make any difference, so use the reference type for now
            
            int innerDimensionLen = T::innerDimensionLen();
            py::buffer_info pInfo = inData.request();
            if(pInfo.size % innerDimensionLen != 0){
                Vk_Logger::RuntimeError(typeid(NoneObj), std::string(typeid(T).name()) + std::string(" array must be of size Nx") + std::to_string(innerDimensionLen) + std::string("!"));
            }
            outLen = static_cast<size_t>(pInfo.size/innerDimensionLen);
            return static_cast<T*>(pInfo.ptr);
        }

        static index_type* indexArrayToCpp(const py::array_t<index_type, py::array::c_style>& inData, size_t& outLen){
            py::buffer_info pInfo = inData.request();
            outLen = static_cast<size_t>(pInfo.size);
            return static_cast<index_type*>(pInfo.ptr);
        }
    };
#endif

	struct Vk_ViewportMargins {
		int32_t left;
		int32_t right;
		int32_t top;
		int32_t bottom;
	};

	struct Vk_Viewport {
		int32_t x;
		int32_t y;
		uint32_t width;
		uint32_t height;
		Vk_RGBColor clearColor;
	};

	enum class Vk_CameraType {
		Rasterizer_IM
	};

	enum class Vk_SteeringType {
		CAMERA_CENTRIC,
		OBJECT_CENTRIC
	};

	struct Vk_CameraCoords {
		std::array<point_type, 3> wPos;
		std::array<point_type, 3> wLook;
		std::array<point_type, 3> wUp;
		std::array<point_type, 3> xAxis;
		std::array<point_type, 3> yAxis;
		std::array<point_type, 3> zAxis;
	};

	struct Vk_CameraSpecs {
		Vk_CameraType type;
		glm::tvec3<point_type> wPos;
		glm::tvec3<point_type> wLook;
		glm::tvec3<point_type> wUp;
		point_type fow;
		point_type wNear;
		point_type wFar;
		Vk_SteeringType steeringType;

#ifdef PYVK
		Vk_CameraSpecs(
			Vk_CameraType p_type,
			const py::array_t<VK4::point_type, py::array::c_style>& p_wPos,
			const py::array_t<VK4::point_type, py::array::c_style>& p_wLook,
			const py::array_t<VK4::point_type, py::array::c_style>& p_wUp,
			point_type p_fow,
			point_type p_wNear,
			point_type p_wFar,
			Vk_SteeringType p_steeringType
		)
		: type(p_type), fow(p_fow), wNear(p_wNear), wFar(p_wFar), steeringType(p_steeringType)
		{
			std::cout << "hello world 1" << std::endl;
			wPos = Vk_NumpyTransformers::arrayToGLMv3(p_wPos);
			std::cout << "hello world 2" << std::endl;
			wLook = Vk_NumpyTransformers::arrayToGLMv3(p_wLook);
			std::cout << "hello world 3" << std::endl;
			wUp = Vk_NumpyTransformers::arrayToGLMv3(p_wUp);
			std::cout << "hello world 4" << std::endl;
		}
#endif
	};

	struct Vk_CameraInit {
		int camId;
		int gridX;
		int gridY;
		Vk_Viewport viewport;
		Vk_CameraSpecs specs;
	};

	struct UnknownCameraTypeException : public std::exception
	{
		const char* what() const throw ()
		{
			return "Unknown camera type";
		}
	};

	enum class Vk_ViewingType {
		LOCAL,
		GLOBAL
	};

	struct Vk_ViewerParams {
		std::string name;
		int width, height, freshPoolSize;
		Vk_ViewingType viewingType;
		std::string screenshotSavePath;

#ifdef PYVK
		Vk_ViewerParams(std::string name, int width, int height, Vk_ViewingType viewingType, int freshPoolSize, std::string screenshotSavePath) 
		: name(name), width(width), height(height), freshPoolSize(freshPoolSize), viewingType(viewingType), screenshotSavePath(screenshotSavePath)
		{}
#else
		Vk_ViewerParams(std::string name, int width, int height) 
		: name(name), width(width), height(height), freshPoolSize(100), viewingType(Vk_ViewingType::GLOBAL), screenshotSavePath("./")
		{}

		Vk_ViewerParams(std::string name, int width, int height, int freshPoolSize) 
		: name(name), width(width), height(height), freshPoolSize(freshPoolSize), viewingType(Vk_ViewingType::GLOBAL), screenshotSavePath("./")
		{}

		Vk_ViewerParams(std::string name, int width, int height, Vk_ViewingType viewingType) 
		: name(name), width(width), height(height), freshPoolSize(100), viewingType(viewingType), screenshotSavePath("./")
		{}

		Vk_ViewerParams(std::string name, int width, int height, std::string screenshotSavePath) 
		: name(name), width(width), height(height), freshPoolSize(100), viewingType(Vk_ViewingType::GLOBAL), screenshotSavePath(screenshotSavePath)
		{}

		Vk_ViewerParams(std::string name, int width, int height, int freshPoolSize, std::string screenshotSavePath) 
		: name(name), width(width), height(height), freshPoolSize(freshPoolSize), viewingType(Vk_ViewingType::GLOBAL), screenshotSavePath(screenshotSavePath)
		{}

		Vk_ViewerParams(std::string name, int width, int height, Vk_ViewingType viewingType, std::string screenshotSavePath) 
		: name(name), width(width), height(height), freshPoolSize(100), viewingType(viewingType), screenshotSavePath(screenshotSavePath)
		{}
#endif
	};

	enum class Vk_BufferUpdateBehaviour {
		GlobalLock, 		 /* one buffer but global lock at data transfer */
		DoubleBuffering,     /* two buffers on GPU at all times */
		LazyDoubleBuffering, /* create a new buffer on update and switch at update time */
		Pinned				 /* use CPU accessible GPU memory for buffer */
	};

	static std::string Vk_BufferUpdateBehaviourToString(Vk_BufferUpdateBehaviour behaviour) {
		switch (behaviour) {
		case Vk_BufferUpdateBehaviour::GlobalLock: return "GlobalLock";
		case Vk_BufferUpdateBehaviour::DoubleBuffering: return "DoubleBuffering";
		case Vk_BufferUpdateBehaviour::LazyDoubleBuffering: return "LazyDoubleBuffering";
		case Vk_BufferUpdateBehaviour::Pinned: return "Pinned";
		default: return "Unknown";
		}
	}

	enum class Vk_BufferSizeBehaviour {
		Init_Empty_Grow_1_5,
		Init_Empty_Grow_2,
		Init_1_0_Grow_1_5,
		Init_1_5_Grow_1_5,
		Init_1_0_Grow_2,
		Init_1_5_Grow_2
	};

	static std::string Vk_BufferSizeBehaviourToString(Vk_BufferSizeBehaviour behaviour) {
		switch (behaviour) {
		case Vk_BufferSizeBehaviour::Init_1_0_Grow_1_5: return "Init_1_0_Grow_1_5";
		case Vk_BufferSizeBehaviour::Init_1_0_Grow_2: return "Init_1_0_Grow_2";
		case Vk_BufferSizeBehaviour::Init_1_5_Grow_1_5: return "Init_1_5_Grow_1_5";
		case Vk_BufferSizeBehaviour::Init_1_5_Grow_2: return "Init_1_5_Grow_2";
		case Vk_BufferSizeBehaviour::Init_Empty_Grow_1_5: return "Init_Empty_Grow_1_5";
		case Vk_BufferSizeBehaviour::Init_Empty_Grow_2: return "Init_Empty_Grow_2";
		default: return "Unknown";
		}
	}

	struct SwapchainSupportDetails {
		VkSurfaceFormatKHR surfaceFormat;
		VkFormat depthFormat;
		VkPresentModeKHR selectedPresentMode;
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
		uint32_t nFramesInFlight;
		bool supportsWideLines;
	};

	struct Screenshot {
		const std::vector<uint8_t>& color;
		int height;
		int width;
	};

	//template<typename T>
	//static T cast(std::map<std::string, py::object>& params, const std::string& name, const std::string& message = "") {
	//	if (strcmp(typeid(T).name(), params.at(name).ptr()->ob_type->tp_name) != 0) {
	//		Vk_Logger::RuntimeError(typeid(this), 
	//			(
	//				std::string("\n************************************************************************\n") +
	//				std::string("Faulty typecast: <") +
	//				GlobalCasters::castHighlightYellow(name) +
	//				std::string("> must be of type <") +
	//				GlobalCasters::castHighlightYellow(std::string(typeid(T).name())) +
	//				std::string("> but has type <") +
	//				GlobalCasters::castHighlightYellow(std::string(params.at(name).ptr()->ob_type->tp_name)) +
	//				std::string(">\n") +
	//				(message.compare("") != 0 ? std::string("*** Message: ") + message : std::string("")) +
	//				std::string(" ***\n************************************************************************\n")
	//			)
	//		);
	//	}
	//	return params.at(name).cast<T>();
	//}

	enum class RenderType {
		Solid,
		Wireframe,
		Point
	};

	static std::string RenderTypeToString(RenderType type) {
		switch (type) {
		case RenderType::Solid:
			return "Solid";
		case RenderType::Wireframe:
			return "Wireframe";
		case RenderType::Point:
			return "Point";
		default:
			return "Unknown";
		}
	}

	enum class Topology {
		Points,
		Lines,
		Triangles
	};

	static std::string TopologyToString(Topology topology) {
		switch (topology) {
		case Topology::Points:
			return "Points";
		case Topology::Lines:
			return "Lines";
		case Topology::Triangles:
			return "Triangles";
		default:
			return "Unknown";
		}
	}

	enum class CullMode {
		NoCulling,
		Back,
		Front
	};

	static std::string CullModeToString(CullMode type) {
		switch (type) {
		case CullMode::NoCulling:
			return "NoCulling";
		case CullMode::Back:
			return "Back";
		case CullMode::Front:
			return "Front";
		default:
			return "Unknown";
		}
	}

#ifdef PYVK
	template<class T>
	static bool tryCast(py::object obj, T& ret) {
		auto incomming = obj.ptr()->ob_type->tp_name;
		if (strcmp(incomming, "str") == 0 && typeid(T) != typeid(std::to_string(0))) return false;
		if (strcmp(incomming, "int") == 0 && typeid(T) != typeid(0)) return false;
		if (strcmp(incomming, "float") == 0 && typeid(T) != typeid(0.0f) && typeid(T) != typeid(0.0)) return false;
		if (strcmp(incomming, "pyvk._vkviewer.lwws_key") == 0 && typeid(T) != typeid(LWWS::LWWS_Key::Special::RandomKey)) return false; 
		if (strcmp(incomming, "pyvk._vkviewer.vk_viewing_type") == 0 && typeid(T) != typeid(Vk_ViewingType)) return false;
		if (strcmp(incomming, "pyvk._vkviewer.vk_camera_type") == 0 && typeid(T) != typeid(Vk_CameraType)) return false;
		if (strcmp(incomming, "pyvk._vkviewer.vk_steering_type") == 0 && typeid(T) != typeid(Vk_SteeringType)) return false;
		if (strcmp(incomming, "pyvk._vkviewer.vk_render_type") == 0 && typeid(T) != typeid(RenderType)) return false;
		if (strcmp(incomming, "pyvk._vkviewer.vk_topology") == 0 && typeid(T) != typeid(Topology)) return false;
		if (strcmp(incomming, "pyvk._vkviewer.vk_cull_mode") == 0 && typeid(T) != typeid(CullMode)) return false;
		if (strcmp(incomming, "pyvk._vkviewer.vk_buffer_characteristics") == 0 && typeid(T) != typeid(Vk_BufferSizeBehaviour)) return false;
		ret = obj.cast<T>();
		return true;
	}

	static int tryCastKey(py::object key){
		std::string kk;
		if(tryCast(key, kk)){
			char c = static_cast<char>(*kk.begin());
			return LWWS::LWWS_Key::KeyToInt(c);
		}

		LWWS::LWWS_Key::Special sk;
		if(tryCast(key, sk)){
			std::cout << "hello world 3.5 " << LWWS::LWWS_Key::SpecialKey2String(sk) << std::endl;
			return LWWS::LWWS_Key::KeyToInt(sk);
		}

		return -1;
	}
#endif

#if defined(PLATFORM_WINDOWS_x64)
	const char PATH_BREAKER = '\\';
#elif defined(PLATFORM_WINDOWS_x32)
	const char PATH_BREAKER = '\\';
#else
	const char PATH_BREAKER = '/';
#endif
}

#if defined(PLATFORM_WINDOWS_x64)

#define FORCEINLINE __forceinline
#define FORCENOINLINE __declspec(noinline)

#ifdef ENGINE_BUILD_LIB
#define ENGINE_API __declspec(dllexport)
#else
#define ENGINE_API __declspec(dllimport)
#endif // ENGINE_BUILD_LIB

#elif defined(PLATFORM_LINUX)
#define FORCEINLINE inline
// TODO: maybe later if ever

#elif defined(PLATFORM_MAC)
#define FORCEINLINE inline

#endif // PLATFORM_WINDOWS

// Create utility methods for assertions
#define ASSERTIONS_ENABLED

#if defined(ASSERTIONS_ENABLED)
#include <iostream>

// define something that assertions can call and return a message
FORCEINLINE void reportAssertionFailure(const char* expression, const char* message, const char* file, int line) {
	std::cerr << "Assertion Failure: " << expression << ", Message: '" << message << "', in File: " << file << ", line: " << line << "\n";
}

#if _MSC_VER
	#include <intrin.h>
	#define debugBreak() __debugbreak();
#else
	#include <signal.h>
	// #define debugBreak() __asm { int3 } // inline assembly code that causes a debug break
	#if defined(SIGTRAP)
		#define debugBreak() raise(SIGTRAP)
	#else
	#	define debugBreak() raise(SIGABRT)
	#endif
#endif

// create assertion macros
// if the assertion expression evaluates to false, call debugBreak()
// #expr gives a string representation of expr

#define VK_CHECK(expr, msg) {\
	ASSERT_MSG(expr == VK_SUCCESS, msg)\
}

#define ASSERT(expr) {\
		if(expr) {\
		} else {\
			reportAssertionFailure(#expr, "", __FILE__, __LINE__);\
			debugBreak();\
		}\
	}

#define ASSERT_MSG(expr, message) {\
		if(expr) {\
		} else {\
			reportAssertionFailure(#expr, message, __FILE__, __LINE__); \
			debugBreak();\
		}\
	}

#if defined(_DEBUG)
#define ASSERT_DEBUG(expr) {\
			if(expr) {\
			} else {\
				reportAssertionFailure(#expr, "", __FILE__, __LINE__);\
				debugBreak();\
			}\
		}

#define ASSERT_DEBUG_MSG(expr, message) {\
		if(expr) {\
		} else {\
			reportAssertionFailure(#expr, message, __FILE__, __LINE__); \
			debugBreak();\
		}\
	}
#else
	// keep the ASSERT_DEBUG defined in release mode, but don't do anything
	#define ASSERT_DEBUG(expr)
	#define ASSERT_DEBUG_MSG(expr, message)
#endif

#else
	// keep asserts defined when ASSERTIONS_ENABLED is not defined, but don't do anything
	#define ASSERT(expr)
	#define ASSERT_MSG(expr, message)
	#define ASSERT_DEBUG(expr)
#endif
