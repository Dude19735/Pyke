#pragma once

#include "../Defines.h"

// namespace py = pybind11;

#include "I_ViewerSteering.h"
#include "Vk_Camera.h"
#include "Vk_ViewerSteeringLib.h"

namespace VK4 {

	class Vk_ViewerSteering_CameraCentric : public I_ViewerSteering {
	public:

		virtual inline Vk_SteeringType vk_type() const {
			return Vk_SteeringType::CAMERA_CENTRIC;
		}

		virtual inline void onMouseAction(Vk_Camera* camera, int px, int py, int dx, int dy, float dz, const std::set<int>& pressedKeys, LWWS::MouseButton mouseButton, LWWS::ButtonOp op, LWWS::MouseAction mouseAction, void* aptr) {
			if(mouseAction == LWWS::MouseAction::MouseMove){
				// const auto& cameras = viewer->vk_cameras();
				if (mouseButton == LWWS::MouseButton::Left && (op == LWWS::ButtonOp::Down || op == LWWS::ButtonOp::SteadyPress)) {
					auto ctrl = LWWS::LWWS_Key::KeyToInt(LWWS::LWWS_Key::Special::LControl);
					if(pressedKeys.find(ctrl) != pressedKeys.end()){
						pan(camera, px, py);
					}
					else {
						rotation(camera, px, py);
					}
				}
			}
			else if(mouseAction == LWWS::MouseAction::MouseScroll){
				zoom(camera, dz);
			}

			// camera->vk_lastMousePosition(xpos, ypos);
		}
		
		virtual inline void onKeyAction(Vk_Camera* camera, int k, LWWS::ButtonOp op, const std::set<int>& otherPressedKeys, void* aptr) {
		}

	private:
		inline void rotation(Vk_Camera* camera, double xpos, double ypos) {
			Vk_ViewerSteeringLib::rotation_CameraCentric(camera, xpos, ypos);
		}

		virtual inline void zoom(Vk_Camera* camera, double dz) {
			Vk_ViewerSteeringLib::zoom(camera, dz);
		}

		virtual inline void pan(Vk_Camera* camera, double xpos, double ypos) {
			Vk_ViewerSteeringLib::pan(camera, xpos, ypos);
		}
	};
}