#pragma once

#include <assert.h>

#include "../Defines.h"

// namespace py = pybind11;

namespace VK4 {

	class Vk_Camera;

	class I_ViewerSteering {
	public:
		static inline glm::tmat3x3<point_type> get3x3Matrix(const glm::tmat4x4<point_type>& matrix) {
			glm::tmat3x3<point_type> temp(
				glm::tvec3<point_type>(matrix[0][0], matrix[0][1], matrix[0][2]),
				glm::tvec3<point_type>(matrix[1][0], matrix[1][1], matrix[1][2]),
				glm::tvec3<point_type>(matrix[2][0], matrix[2][1], matrix[2][2])
			);
			return temp;
		}

		virtual inline Vk_SteeringType vk_type() const = 0;
		virtual inline void onMouseAction(Vk_Camera* camera, int px, int py, int dx, int dy, float dz, const std::set<int>& pressedKeys, LWWS::MouseButton mouseButton, LWWS::ButtonOp op, LWWS::MouseAction mouseAction, void* aptr) = 0;
		virtual inline void onKeyAction(Vk_Camera* camera, int k, LWWS::ButtonOp op, const std::set<int>& otherPressedKeys, void* aptr) = 0;

	private:
		virtual inline void rotation(Vk_Camera* camera, double dx, double dy) = 0;
		virtual inline void zoom(Vk_Camera* camera, double dz) = 0;
		virtual inline void pan(Vk_Camera* camera, double xpos, double ypos) = 0;
	};
}