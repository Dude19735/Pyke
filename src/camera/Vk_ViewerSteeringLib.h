#pragma once

#include "../Defines.h"

#include "Vk_Camera.h"

namespace VK4 {
    class Vk_ViewerSteeringLib {
        public:
        static inline void rotation_CameraCentric(Vk_Camera* camera, double xpos, double ypos) {
			point_type dx = static_cast<point_type>((xpos - camera->vk_lastMousePosX()) / static_cast<point_type>(camera->vk_viewport().height));
			point_type dy = static_cast<point_type>((ypos - camera->vk_lastMousePosY()) / static_cast<point_type>(camera->vk_viewport().width));

			// isolate rotation matrix
			glm::tmat3x3<point_type> tempView = I_ViewerSteering::get3x3Matrix(camera->vk_view());
			// set new lookAt vector in camera coordinates
			glm::tvec3<point_type> cnLookAt = glm::vec3(-dx, -dy, 1);
			// transform new lookAt vector into world coordinates (note: tempView is orthogonal)
			glm::tvec3<point_type> wnLookAt = glm::normalize(glm::transpose(tempView) * cnLookAt);
			// get the old lookAt vector (z vector from tempView == old z-axis)
			// glm::tvec3<point_type> lookAt = glm::row(tempView, 2);

			// calculate the new x axis direction
			glm::tvec3<point_type> newX = glm::cross(wnLookAt, camera->vk_wUp());

			//! this is only init of camera => crashing is allowed
			// assert(glm::l2Norm(newX) > 0 && "wUp and lookAt are collinear");

			// calculate the new y axis
			glm::tvec3<point_type> newY = glm::cross(wnLookAt, newX);
			glm::tmat3x3<point_type> xyzT = glm::transpose(glm::mat3(glm::normalize(newX), glm::normalize(newY), wnLookAt));

			camera->vk_view(glm::mat4(
				glm::tvec4<point_type>(xyzT[0], 0.0),
				glm::tvec4<point_type>(xyzT[1], 0.0),
				glm::tvec4<point_type>(xyzT[2], 0.0),
				glm::tvec4<point_type>(-(xyzT * camera->vk_wPos()), 1.0)
			));

			// calculate the distance between the camera position and the old wLookAt
			point_type dist = glm::l2Norm(camera->vk_wPos() - camera->vk_wLook());
			// set the new lookAt point for the camera at the same distance in direction of the
			// new look direction.
			camera->vk_wLook(dist*wnLookAt + camera->vk_wPos());

			// set the camera new up direction to the negative of the new y axis
			camera->vk_wUp(-glm::normalize(newY));
		}

        static inline void rotation_ObjectCentric(Vk_Camera* camera, double xpos, double ypos) {
			point_type dx = static_cast<point_type>((xpos - camera->vk_lastMousePosX()) / static_cast<point_type>(camera->vk_viewport().height));
			point_type dy = static_cast<point_type>((ypos - camera->vk_lastMousePosY()) / static_cast<point_type>(camera->vk_viewport().width));

			// calculate the distance between the camera position and the old wLookAt
			point_type dist = glm::l2Norm(camera->vk_wPos() - camera->vk_wLook());

			// isolate rotation matrix
			glm::tmat3x3<point_type> tempView = I_ViewerSteering::get3x3Matrix(camera->vk_view());

			// get new looking direction based on dy and dy
			glm::tvec3<point_type> cnLookAt = glm::vec3(dx, dy, 1);
			// transform new looking direction into world coordinates
			glm::tvec3<point_type> wnLookAt = glm::normalize(glm::transpose(tempView) * cnLookAt);

			// calculate new place of camera based on camera lookAt point and new looking direction
			glm::tvec3<point_type> wnCameraPos = camera->vk_wLook() - dist*wnLookAt;
			
			// calculate the new x axis direction
			glm::tvec3<point_type> newX = glm::cross(wnLookAt, camera->vk_wUp());
			glm::tvec3<point_type> newY = glm::cross(wnLookAt, newX);
			glm::tmat3x3<point_type> xyzT = glm::transpose(glm::mat3(glm::normalize(newX), glm::normalize(newY), wnLookAt));

			camera->vk_wPos(wnCameraPos);

			// set the new lookAt point for the camera at the same distance in direction of the
			// new look direction.
			camera->vk_wLook(dist*wnLookAt + wnCameraPos);

			// set the camera new up direction to the negative of the new y axis
			camera->vk_wUp(-glm::normalize(newY));

			camera->vk_view(glm::mat4(
				glm::tvec4<point_type>(xyzT[0], 0.0),
				glm::tvec4<point_type>(xyzT[1], 0.0),
				glm::tvec4<point_type>(xyzT[2], 0.0),
				glm::tvec4<point_type>(-(xyzT * wnCameraPos), 1.0)
			));
		}

        static inline void zoom(Vk_Camera* camera, double dz) {
			// isolate rotation matrix
			glm::tmat3x3<point_type> tempView = I_ViewerSteering::get3x3Matrix(camera->vk_view());
			glm::tvec3<point_type> lookAt = glm::row(tempView, 2);

			// calculate the new position
			// only use the xoffset since the mouse wheel can only act vertically
			// std::cout << xoffset << " " << yoffset << std::endl;
			const point_type acc = 1.0f; // 0.25e-1f;
			camera->vk_wPos(camera->vk_wPos() + acc * dz * lookAt);

			// write back into the view matrix
			glm::tvec3<point_type> newTranslation = -(tempView * camera->vk_wPos());
			camera->vk_view(glm::tmat4x4<point_type>(
				glm::column(camera->vk_view(), 0),			/*new x axis*/
				glm::column(camera->vk_view(), 1),			/*new y axis*/
				glm::column(camera->vk_view(), 2),			/*new z axis*/
				glm::tvec4<point_type>(newTranslation, 1.0)	/*new position of the camera*/
			));
		}

        static inline void pan(Vk_Camera* camera, double xpos, double ypos) {
			glm::tmat3x3<point_type> tempView = I_ViewerSteering::get3x3Matrix(camera->vk_view());
			glm::tvec3<point_type> xAxis = -glm::row(tempView, 0);
			glm::tvec3<point_type> yAxis = -glm::row(tempView, 1);

			// do the offset kind of dependent on the amount of mouse move
			point_type dx = static_cast<point_type>((xpos - camera->vk_lastMousePosX()) / static_cast<point_type>(camera->vk_viewport().height));
			point_type dy = static_cast<point_type>((ypos - camera->vk_lastMousePosY()) / static_cast<point_type>(camera->vk_viewport().width));

			const point_type acc = static_cast<point_type>(
				5 * std::sqrt(camera->vk_wPos().x * camera->vk_wPos().x + camera->vk_wPos().y * camera->vk_wPos().y + camera->vk_wPos().z * camera->vk_wPos().z) * 10e-2);

			glm::tvec3<point_type> offset = acc * (dx * xAxis + dy * yAxis);
			camera->vk_wPos(camera->vk_wPos() + offset);
			camera->vk_wLook(camera->vk_wLook() + offset);

			// write back into the view matrix
			glm::vec3 newTranslation = -(tempView * camera->vk_wPos());
			camera->vk_view(glm::tmat4x4<point_type>(
				glm::column(camera->vk_view(), 0),			/*new x axis*/
				glm::column(camera->vk_view(), 1),			/*new y axis*/
				glm::column(camera->vk_view(), 2),			/*new z axis*/
				glm::tvec4<point_type>(newTranslation, 1.0)	/*new position of the camera*/
			));
		}
    };
}