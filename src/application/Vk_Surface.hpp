#pragma once

#include <array>

#include "../Defines.h"
#include "Vk_Instance.hpp"

namespace VK4 {

	class Vk_Surface {
	public:
		Vk_Surface(
			Vk_Instance* instance, 
			std::string title, 
			int width, 
			int height,
			bool resizable,
            bool disableMousePointerOnHover=false,
            int hoverTimeoutMS=500
			) 
			:
			_window(nullptr), 
			_instance(instance), 
			_reason(""),
			_title(title),
			_windowWidth(width),
			_windowHeight(height),
			_resizable(resizable),
			_disableMousePointerOnHover(disableMousePointerOnHover),
			_hoverTimeoutMS(hoverTimeoutMS),
			_surface(nullptr)
			{
			Vk_Logger::Log(typeid(this), GlobalCasters::castConstructorTitle("Create Surface"));
#if defined(VK_USE_PLATFORM_WIN32_KHR)
			_window = std::make_unique<LWWS::LWWS_Window_Win>(_title, _windowWidth, _windowHeight, _resizable, _disableMousePointerOnHover, _hoverTimeoutMS, "DesktopApp2", false);
#elif defined(VK_USE_PLATFORM_XCB_KHR) || defined(VK_USE_PLATFORM_XLIB_KHR)
			_window = std::make_unique<LWWS::LWWS_Window_X11>(_title, _windowWidth, _windowHeight, _resizable, _disableMousePointerOnHover, _hoverTimeoutMS, false);
#endif
			VK_CHECK(createVulkanWindowSurface(_instance->vk_instance(), nullptr, &_surface), "Failed to create Vulkan surface!");
		}

		~Vk_Surface()
		{
			Vk_Logger::Log(typeid(this), GlobalCasters::castDestructorTitle(
				std::string("Destroy Surface") + (_reason.compare("") == 0 ? "" : std::string(" (") + _reason + std::string(")"))
			));
			vkDestroySurfaceKHR(_instance->vk_instance(), _surface, nullptr);
		}

		const VkSurfaceKHR vk_surface() const {
			return _surface;
		}

		const VkExtent2D vk_canvasSize() const {
			int w,h;
			_window->canvasSize(w, h);
			return  VkExtent2D{
				.width=static_cast<uint32_t>(w),
				.height=static_cast<uint32_t>(h)
			};
		}

		const VkExtent2D vk_canvasOriginalSize() const {
			int w,h;
			_window->canvasInitSize(w, h);
			return  VkExtent2D{
				.width=static_cast<uint32_t>(w),
				.height=static_cast<uint32_t>(h)
			};
		}

		const VkExtent2D vk_frameSize() const {
			int w,h;
			_window->frameSize(w, h);
			return  VkExtent2D{
				.width=static_cast<uint32_t>(w),
				.height=static_cast<uint32_t>(h)
			};
		}

		LWWS::LWWS_Window* vk_lwws_window() const {
			return _window.get();
		}

	private:
		// GLFWwindow* _window;
		std::unique_ptr<LWWS::LWWS_Window> _window;
		Vk_Instance* _instance;
		std::string _reason;

		std::string _title;
		int _windowWidth;
		int _windowHeight;
		bool _resizable;
		bool _disableMousePointerOnHover;
		int _hoverTimeoutMS;

		VkSurfaceKHR _surface;

#if defined(VK_USE_PLATFORM_WIN32_KHR)
		VkResult createVulkanWindowSurface(VkInstance instance, const VkAllocationCallbacks* allocator, VkSurfaceKHR* surface)
        {
            VkResult err;
            VkWin32SurfaceCreateInfoKHR sci;
            PFN_vkCreateWin32SurfaceKHR vkCreateWin32SurfaceKHR;

            vkCreateWin32SurfaceKHR = reinterpret_cast<PFN_vkCreateWin32SurfaceKHR>(vkGetInstanceProcAddr(instance, "vkCreateWin32SurfaceKHR"));
            if (!vkCreateWin32SurfaceKHR) return VK_ERROR_EXTENSION_NOT_PRESENT;

			LWWS::LWWS_Window_Win* window = reinterpret_cast<LWWS::LWWS_Window_Win*>(_window.get());
			HWND hWnd;
			HINSTANCE hInstance;
			window->getWin32WindowDescriptors(hWnd, hInstance);

            memset(&sci, 0, sizeof(sci));
            sci.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
            sci.hinstance = hInstance;
            sci.hwnd = hWnd;

            err = vkCreateWin32SurfaceKHR(instance, &sci, allocator, surface);

            return err;
        }
#elif defined(VK_USE_PLATFORM_XCB_KHR)
		VkResult createVulkanWindowSurface(VkInstance instance, const VkAllocationCallbacks* allocator, VkSurfaceKHR* surface){
			VkResult err;
			VkXcbSurfaceCreateInfoKHR sci;
			PFN_vkCreateXcbSurfaceKHR vkCreateXcbSurfaceKHR;

			LWWS::LWWS_Window_X11* window = reinterpret_cast<LWWS::LWWS_Window_X11*>(_window.get());
			xcb_window_t screenId;
			Display* display;
			window->getX11XcbWindowDescriptors(display, screenId);

			xcb_connection_t* connection = XGetXCBConnection(display);
			if (!connection)
			{
				return VK_ERROR_EXTENSION_NOT_PRESENT;
			}

			vkCreateXcbSurfaceKHR = (PFN_vkCreateXcbSurfaceKHR)vkGetInstanceProcAddr(instance, "vkCreateXcbSurfaceKHR");
			if (!vkCreateXcbSurfaceKHR)
			{
				return VK_ERROR_EXTENSION_NOT_PRESENT;
			}

			memset(&sci, 0, sizeof(sci));
			sci.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
			sci.connection = connection;
			sci.window = screenId;

			err = vkCreateXcbSurfaceKHR(instance, &sci, allocator, surface);
			return err;
		}
#elif defined(VK_USE_PLATFORM_XLIB_KHR)
		VkResult createVulkanWindowSurface(VkInstance instance, const VkAllocationCallbacks* allocator, VkSurfaceKHR* surface){
			VkResult err;
			VkXlibSurfaceCreateInfoKHR sci;
			PFN_vkCreateXlibSurfaceKHR vkCreateXlibSurfaceKHR;

			vkCreateXlibSurfaceKHR = (PFN_vkCreateXlibSurfaceKHR)vkGetInstanceProcAddr(instance, "vkCreateXlibSurfaceKHR");
			if (!vkCreateXlibSurfaceKHR)
			{
				return VK_ERROR_EXTENSION_NOT_PRESENT;
			}

			LWWS::LWWS_Window_X11* lwwsWindow = reinterpret_cast<LWWS::LWWS_Window_X11*>(_window.get());
			Window window;
			Display* display;
			lwwsWindow->getX11XlibWindowDescriptors(display, window);

			memset(&sci, 0, sizeof(sci));
			sci.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
			sci.dpy = display;
			sci.window = window;

			err = vkCreateXlibSurfaceKHR(instance, &sci, allocator, surface);
			return err;
		}
#else
		VkResult createVulkanWindowSurface(VkInstance instance, const VkAllocationCallbacks* allocator, VkSurfaceKHR* surface){
			Vk_Logger::RuntimeError(typeid(this), "Unsupported platform. Can't create Vulkan surface!");
		}
#endif
	};
}
