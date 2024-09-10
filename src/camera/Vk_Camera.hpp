#pragma once

#include <vector>
#include "../Defines.h"

#include "../renderer/I_Renderer.hpp"
#include "I_ViewerSteering.hpp"
// namespace py = pybind11;

namespace VK4 {
    class Vk_Camera {
        public:

        Vk_Camera(const Vk_CameraInit& init, std::unique_ptr<I_ViewerSteering> steering, int currentParentWindowWidth, int currentParentWindowHeight)
        : 
        _viewportId(init.viewportId),
        _gridX(init.gridX),
        _gridY(init.gridY),
        _type(init.specs.type),
        _renderer(nullptr),
        _viewport(init.viewport),
        _originalWidth(init.viewport.width), 
        _originalHeight(init.viewport.height), 
        _originalOffsetX(init.viewport.x), 
        _originalOffsetY(init.viewport.y),
        _currentParentWindowWidth(currentParentWindowWidth),
        _currentParentWindowHeight(currentParentWindowHeight),
        _wPos(init.specs.wPos),
        _wLook(init.specs.wLook),
        _wUp(init.specs.wUp),
        _fow(init.specs.fow),
        _wNear(init.specs.wNear),
        _wFar(init.specs.wFar),
        _view(glm::mat4(1.0f)),
        _perspective(glm::mat4(1.0f)),
        _xAxis(glm::vec3(0.0f, 0.0f, 0.0f)),
        _yAxis(glm::vec3(0.0f, 0.0f, 0.0f)),
        _zAxis(glm::vec3(0.0f, 0.0f, 0.0f)),
        _ctrlPressed(false),
        _lastMousePosX(0.0),
        _lastMousePosY(0.0),
        _aspect(1.0f),
        _pv({}),
        _xResizeRoundingIndicator(0),
        _yResizeRoundingIndicator(0),
        _steering(nullptr)
        {
            _steering = std::move(steering);
			vk_calculateTransform();
		}

		inline void vk_calculateTransform() {
            _aspect = static_cast<point_type>(_viewport.width) / static_cast<point_type>(_viewport.height);
			point_type f1 = _wFar / (_wFar - _wNear);
			point_type f2 = -(_wNear * _wFar) / (_wFar - _wNear);
			point_type x = 1 / glm::tan(_fow / 2);
			point_type y = _aspect / glm::tan(_fow / 2);

			point_type proj[16] = {
				x, 0, 0, 0,
				0, y, 0, 0,
				0, 0,f1,f2,
				0, 0, 1, 0
			};
			_perspective = glm::transpose(glm::make_mat4(proj));

			glm::tvec3<point_type> lookAt = glm::normalize(_wLook - _wPos);
			glm::tvec3<point_type> newX = glm::cross(lookAt, _wUp);

			//! this is only init of camera => crashing is allowed
			assert(glm::l2Norm(newX) > 0 && "wUp and lookAt are collinear");

			glm::tvec3<point_type> newY = glm::cross(lookAt, newX);
			glm::tmat3x3<point_type> xyzT = glm::transpose(
                glm::mat3(
                    glm::normalize(newX), 
                    glm::normalize(newY), 
                    lookAt
                )
            );

			_view = glm::mat4(
				glm::tvec4<point_type>(xyzT[0], 0.0),
				glm::tvec4<point_type>(xyzT[1], 0.0),
				glm::tvec4<point_type>(xyzT[2], 0.0),
				glm::tvec4<point_type>(-(xyzT * _wPos), 1.0)
			);

            _xAxis = newX;
            _yAxis = newY;
            _zAxis = lookAt;
		}

        bool vk_moveCamera(int32_t xNew, int32_t yNew) {
			if (xNew < 0 && yNew < 0) return false;
			else if (xNew < 0) {
				// move xNew proportionally according to the movement of yNew
				float dy = static_cast<float>(yNew - _viewport.y);
				float py = dy / static_cast<float>(_currentParentWindowHeight);

				float dx = py * _currentParentWindowWidth;
				int32_t idx = static_cast<int32_t>(dx);
				if (idx != dy) idx += _xResizeRoundingIndicator;

				xNew = _viewport.x + idx;
				_xResizeRoundingIndicator++;
				_xResizeRoundingIndicator %= 2;
			}
			else if (yNew < 0) {
				// move yNew proportionally according to the movement of xNew
				float dx = static_cast<float>(xNew - _viewport.x);
				float px = dx / static_cast<float>(_currentParentWindowWidth);
				
				float dy = px * _currentParentWindowHeight;
				int32_t idy = static_cast<int32_t>(dy);
				if(idy != dy) idy += _yResizeRoundingIndicator;
				
				yNew = _viewport.y + idy;
				_yResizeRoundingIndicator++;
				_yResizeRoundingIndicator %= 2;
			}

			if (static_cast<int32_t>(_viewport.width) + xNew > static_cast<int32_t>(_currentParentWindowWidth)) {
				return false;
			}
			if (static_cast<int32_t>(_viewport.height) + yNew > static_cast<int32_t>(_currentParentWindowHeight)) {
				return false;
			}

			_viewport.x = xNew;
			_viewport.y = yNew;

			return true;
		}

		bool vk_scaleCamera(LWWS::TViewportId viewportId, int32_t sxNew, int32_t syNew) {
			if (sxNew < 0 && syNew < 0) return false;
			else if (sxNew < 0) {
				// move xNew proportionally according to the movement of yNew
				float aspect = static_cast<float>(_viewport.width)/static_cast<float>(_viewport.height);
				float fsxNew = aspect * syNew;
				sxNew = static_cast<int32_t>(fsxNew);
				if (sxNew != fsxNew) sxNew += _xResizeRoundingIndicator;

				_xResizeRoundingIndicator++;
				_xResizeRoundingIndicator %= 2;
			}
			else if (syNew < 0) {
				// move yNew proportionally according to the movement of xNew
				float aspect = static_cast<float>(_viewport.width) / static_cast<float>(_viewport.height);
				float fsyNew = sxNew / aspect;
				syNew = static_cast<int32_t>(fsyNew);
				if (syNew != fsyNew) syNew += _yResizeRoundingIndicator;

				_yResizeRoundingIndicator++;
				_yResizeRoundingIndicator %= 2;
			}

			if (_viewport.x + sxNew > static_cast<int32_t>(_currentParentWindowWidth)) {
				return false;
			}
			if (_viewport.y + syNew > static_cast<int32_t>(_currentParentWindowHeight)) {
				return false;
			}

			_viewport.width = sxNew;
			_viewport.height = syNew;

			return true;
		}

        inline void vk_update(const uint32_t imageIndex) {
            _pv.mat = _perspective * _view;
            _renderer->vk_update(imageIndex, _pv);
        }

        inline bool vk_contains(const double x, const double y){
            bool b1 = _viewport.x <= x && x <= _viewport.x + _viewport.width;
            bool b2 = _viewport.y <= y && y <= _viewport.y + _viewport.height;
            return b1 && b2;
        }

        /* == Setter == */
        inline void vk_renderer(std::unique_ptr<I_Renderer> renderer){
            _renderer = std::move(renderer);
        }

        inline void vk_parentWindowSize(int currentParentWindowWidth, int currentParentWindowHeight){
            _currentParentWindowWidth = currentParentWindowWidth;
            _currentParentWindowHeight = currentParentWindowHeight;
        }

        inline void vk_viewport(const Vk_Viewport& viewport) { // int32_t x, int32_t y, uint32_t width, uint32_t height){
            _viewport.x = viewport.x;
            _viewport.y = viewport.y;
            _viewport.width = viewport.width;
            _viewport.height = viewport.height;
        }

        inline void vk_mvp(const glm::mat4& mvp){
            _pv.mat = mvp;
        }

        inline void vk_view(const glm::mat4& view){
            _view = view;
        }

        inline void vk_wLook(const glm::tvec3<point_type>& wLook){
            _wLook = wLook;
        }

        inline void vk_wPos(const glm::tvec3<point_type>& wPos){
            _wPos = wPos;
        }

        inline void vk_wUp(const glm::tvec3<point_type>& wUp){
            _wUp = wUp;
        }

        inline void vk_lastMousePosition(double lastMousePosX, double lastMousePosY){
            _lastMousePosX = lastMousePosX;
            _lastMousePosY = lastMousePosY;
        }

        /* == Getter == */
        inline const LWWS::TViewportId vk_viewportId() const { return _viewportId; }
        inline const int vk_gridX() const { return _gridX; }
        inline const int vk_gridY() const { return _gridY; }
        inline const Vk_CameraType vk_type() const { return _type; }
        inline const UniformBufferType_RendererMat4& vk_mvp() const { return _pv; }
        inline const int vk_originalX() const { return _originalOffsetX; }
        inline const int vk_originalY() const { return _originalOffsetY; }
        inline const int vk_originalWidth() const { return _originalWidth; }
        inline const int vk_originalHeight() const { return _originalHeight; }
        inline const Vk_Viewport& vk_viewport() const { return _viewport; }
        inline I_Renderer* const vk_renderer() const { return _renderer.get(); }
        inline const I_ViewerSteering* vk_steering() const { return _steering.get(); }
        inline const double vk_lastMousePosX() const { return _lastMousePosX; }
        inline const double vk_lastMousePosY() const { return _lastMousePosY; }
        inline const glm::tvec3<point_type>& vk_wPos() const { return _wPos; }
        inline const glm::tvec3<point_type>& vk_wLook() const { return _wLook; }
        inline const glm::tvec3<point_type>& vk_wUp() const { return _wUp; }
        inline const point_type vk_fow() const { return _fow; }
        inline const point_type vk_wNear() const { return _wNear; }
        inline const point_type vk_wFar() const { return _wFar; }
        inline const glm::mat4& vk_view() const { return _view; }
        inline const glm::mat4& vk_perspective() const { return _perspective; }
        
        inline const Vk_CameraCoords vk_cameraCoords() const {
            return Vk_CameraCoords {
                .wPos = std::array<point_type, 3> {_wPos.x, _wPos.y, _wPos.z},
                .wLook = std::array<point_type, 3> {_wLook.x, _wLook.y, _wLook.z},
                .wUp = std::array<point_type, 3> {_wUp.x, _wUp.y, _wUp.z},
                .xAxis = std::array<point_type, 3> {_xAxis.x, _xAxis.y, _xAxis.z},
                .yAxis = std::array<point_type, 3> {_yAxis.x, _yAxis.y, _yAxis.z},
                .zAxis = std::array<point_type, 3> {_zAxis.x, _zAxis.y, _zAxis.z}
            };
        }

        /* == Callbacks == */
        inline void onMouseAction(int px, int py, int dx, int dy, float dz, const std::set<int>& pressedKeys, LWWS::MouseButton mouseButton, LWWS::ButtonOp op, LWWS::MouseAction mouseAction, void* aptr) {
			_steering->onMouseAction(this, px, py, dx, dy, dz, pressedKeys, mouseButton, op, mouseAction, aptr);
		}

		inline void onKeyAction(int k, LWWS::ButtonOp op, const std::set<int>& otherPressedKeys, void* aptr) {
			_steering->onKeyAction(this, k, op, otherPressedKeys, aptr);
		}

        // inline void onMouseMoveCallback(GLFWwindow* window, double xpos, double ypos) {
		// 	_steering->onMouseMove(window, this, xpos, ypos);
		// }

		// inline void onMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset){
		// 	_steering->onMouseScroll(window, this, yoffset, yoffset);
		// }

		// inline void onKeyPressedCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
		// 	_steering->onKeyPressed(window, this, key, scancode, action, mods);
		// }

        private:

        LWWS::TViewportId _viewportId;
        int _gridX;
        int _gridY;
        Vk_CameraType _type;
        std::unique_ptr<I_Renderer> _renderer;
        Vk_Viewport _viewport;
 
        const int _originalWidth;
        const int _originalHeight;
        const int _originalOffsetX;
        const int _originalOffsetY;

        int _currentParentWindowWidth;
        int _currentParentWindowHeight;
 
        glm::tvec3<point_type> _wPos;
        glm::tvec3<point_type> _wLook;
        glm::tvec3<point_type> _wUp;
        point_type _fow;
        point_type _wNear;
        point_type _wFar;
        glm::mat4 _view;
        glm::mat4 _perspective;

        glm::tvec3<point_type> _xAxis;
        glm::tvec3<point_type> _yAxis;
        glm::tvec3<point_type> _zAxis;
 
        bool _ctrlPressed;
        double _lastMousePosX;
        double _lastMousePosY;
        float _aspect;
 
        UniformBufferType_RendererMat4 _pv;
        int _xResizeRoundingIndicator;
        int _yResizeRoundingIndicator;

        std::unique_ptr<I_ViewerSteering> _steering;
    };
}