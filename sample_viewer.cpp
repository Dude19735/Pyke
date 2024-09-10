#include <math.h>
#include "./src/Vk_SampleObjects.hpp"
#include "./src/Vk_ColorOp.hpp"
#include "./src/camera/Vk_GridLayout.hpp"
#include "./src/Vk_Viewer.hpp"

/**
 * Check out test_py/test_vkviewer.py to get a commented example. The functionalities depicted in the two examples is identical
 */

class Viewer {
	public:
	Viewer() : _angle(0.0f), _size(1.0f), _step(0.1f), _on(false) {
		std::string name = "Pyke";
		uint32_t width = 1024;
		uint32_t height = 800;
		_device = std::make_unique<VK4::Vk_Device>(name, VK4::Vk_DevicePreference::USE_DISCRETE_GPU);
		_cam = std::make_unique<VK4::Vk_Viewer>(_device.get(), VK4::Vk_ViewerParams(name, width, height, VK4::Vk_ViewingType::LOCAL, "."));

		VK4::Vk_CameraSpecs specs_ObjectCentric {
			.type = VK4::Vk_CameraType::Rasterizer_IM,
			.wPos = glm::tvec3<VK4::point_type> {5.0f, 5.0f, 5.0f },
			.wLook = glm::tvec3<VK4::point_type> {0.0f, 0.0f, 0.0f },
			.wUp = glm::tvec3<VK4::point_type> {0.0f, 0.0f, 1.0f },
			.fow = 80.0f / 180.0f * glm::pi<float>(),
			.wNear = 1.0f,
			.wFar = 100.0f,
			.steeringType = VK4::Vk_SteeringType::OBJECT_CENTRIC
		};

		VK4::Vk_CameraSpecs specs_CameraCentric {
			.type = VK4::Vk_CameraType::Rasterizer_IM,
			.wPos = glm::tvec3<VK4::point_type> {5.0f, 5.0f, 5.0f },
			.wLook = glm::tvec3<VK4::point_type> {0.0f, 0.0f, 0.0f },
			.wUp = glm::tvec3<VK4::point_type> {0.0f, 0.0f, 1.0f },
			.fow = 80.0f / 180.0f * glm::pi<float>(),
			.wNear = 1.0f,
			.wFar = 100.0f,
			.steeringType = VK4::Vk_SteeringType::CAMERA_CENTRIC
		};

		VK4::Vk_RGBColor from { .r=0.5f, .g=0.0f, .b=0.0f };
		VK4::Vk_RGBColor to { .r=0.0f, .g=0.0f, .b=0.5f };

		VK4::Vk_GridLayout layout(3, 3, 0, 0);
		layout.vk_addCamera(0,0, {specs_ObjectCentric, VK4::Vk_ColorOp::rgb_lerp(0.000f, from, to)});
		layout.vk_addCamera(0,1, {specs_CameraCentric, VK4::Vk_ColorOp::rgb_lerp(0.125f, from, to)});
		layout.vk_addCamera(0,2, {specs_ObjectCentric, VK4::Vk_ColorOp::rgb_lerp(0.250f, from, to)});
		layout.vk_addCamera(1,0, {specs_CameraCentric, VK4::Vk_ColorOp::rgb_lerp(0.375f, from, to)});
		layout.vk_addCamera(1,1, {specs_ObjectCentric, VK4::Vk_ColorOp::rgb_lerp(0.500f, from, to)});
		layout.vk_addCamera(1,2, {specs_CameraCentric, VK4::Vk_ColorOp::rgb_lerp(0.625f, from, to)});
		layout.vk_addCamera(2,0, {specs_ObjectCentric, VK4::Vk_ColorOp::rgb_lerp(0.750f, from, to)});
		layout.vk_addCamera(2,1, {specs_CameraCentric, VK4::Vk_ColorOp::rgb_lerp(0.875f, from, to)});
		layout.vk_addCamera(2,2, {specs_ObjectCentric, VK4::Vk_ColorOp::rgb_lerp(1.000f, from, to)});

		_cam->vk_addCamera(layout.vk_layoutList(width, height));

		_pointSize = 10.0f;
		_lineWidth = 5.0f;
		_alpha = 0.5f;

		auto cp = VK4::Vk_SampleObjects::Point_P();
		auto cc = VK4::Vk_SampleObjects::Point_C();
		auto ci = VK4::Vk_SampleObjects::Point_P_C_Indices();
		_dot = VK4::S_Dot_P_C::create(
			_device.get(),
			"test_object",
			glm::tmat4x4<VK4::point_type> {1,0,0,0, 0,1,0,0, 0,0,1,0, 1.5,1.5,0,1},
			cp, cc, ci, _pointSize, _alpha,
			VK4::Vk_BufferUpdateBehaviour::Staged_GlobalLock
		);

		_line = VK4::S_Line_P_C::create(
			_device.get(),
			"test_line_obj",
			glm::tmat4x4<VK4::point_type> {1,0,0,0, 0,1,0,0, 0,0,1,0, -1.5,1.5,0,1},
			VK4::Vk_SampleObjects::Line_P(), VK4::Vk_SampleObjects::Line_C(), VK4::Vk_SampleObjects::Line_P_C_Indices(),
			_lineWidth, 1.0f,
			VK4::Vk_BufferUpdateBehaviour::Direct_GlobalLock
		);

		float f=-4.0f;
		float t= 4.0f;
		float l= 0.5f;
		_coords = VK4::S_Line_P_C::create(
			_device.get(),
			"coords",
			glm::tmat4x4<VK4::point_type> {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1},
			VK4::Vk_SampleObjects::Coords_P(f,t,l, f,t,l, f,t,l), 
			VK4::Vk_SampleObjects::Coords_C(1.0f, 1.0f, 1.0f), 
			VK4::Vk_SampleObjects::Coords_P_C_Indices(),
			2.0f, 1.0f
		);

		_mesh = VK4::S_Mesh_P_C::create(
			_device.get(),
			"mesh",
			glm::tmat4x4<VK4::point_type> {1,0,0,0, 0,1,0,0, 0,0,1,0, -1.5,-1.5,0,1},
			VK4::Vk_SampleObjects::Cube1_P(),
			VK4::Vk_SampleObjects::Cube1_C(),
			VK4::Vk_SampleObjects::Cube1_P_C_Indices(),
			1.0f,
			VK4::CullMode::Back,
			VK4::RenderType::Solid,
			1.0f, 1.0f,
			VK4::Vk_BufferUpdateBehaviour::Staged_GlobalLock
		);

		_mesh2 = VK4::S_Mesh_P_C::create(
			_device.get(),
			"mesh2",
			glm::tmat4x4<VK4::point_type> {1,0,0,0, 0,1,0,0, 0,0,1,0, 1.5,-1.5,0,1},
			VK4::Vk_SampleObjects::Cube2_P(_angle),
			VK4::Vk_SampleObjects::Cube2_C(),
			VK4::Vk_SampleObjects::Cube2_P_C_N_Indices(),
			1.0f,
			VK4::CullMode::Back,
			VK4::RenderType::Wireframe,
			1.0f, 1.0f,
			VK4::Vk_BufferUpdateBehaviour::Staged_GlobalLock
		);

		_mesh2Normals = VK4::S_Line_P_C::create(
			_device.get(),
			"mesh2_normals",
			glm::tmat4x4<VK4::point_type> {1,0,0,0, 0,1,0,0, 0,0,1,0, 1.5,-1.5,0,1},
			VK4::Vk_SampleObjects::Cube2_NormalLines_P(0.5f, _angle), VK4::Vk_SampleObjects::Cube2_NormalLines_C(), VK4::Vk_SampleObjects::Cube2_NormalLines_Indices(),
			_lineWidth, 1.0f,
			VK4::Vk_BufferUpdateBehaviour::Staged_GlobalLock
		);

		_cam->vk_registerAction('r', this, &Viewer::rotate);
		_cam->vk_registerAction('s', this, &Viewer::scale);
		_cam->vk_registerAction('o', this, &Viewer::onoff);
		_cam->vk_registerAction('i', this, &Viewer::incr_p_size);
		_cam->vk_registerAction('d', this, &Viewer::decr_p_size);
		_cam->vk_registerAction('z', this, &Viewer::decr_alpha);
		_cam->vk_registerAction('x', this, &Viewer::incr_alpha);
		_cam->vk_registerAction('1', this, &Viewer::decr_l_width);
		_cam->vk_registerAction('2', this, &Viewer::incr_l_width);

		_cam->vk_runThread();

		_cam->vk_attachToAll(_dot);
		_cam->vk_attachToAll(_line);
		_cam->vk_attachToAll(_coords);
		_cam->vk_attachToAll(_mesh);
		_cam->vk_attachToAll(_mesh2);
		_cam->vk_attachToAll(_mesh2Normals);
		_cam->vk_rebuildAndRedraw();

		/* empty space to do stuff */
		int counter = 0;
		while(_cam->vk_running()){
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
			std::cout << "hello world: " << counter++ << std::endl;
		}
		std::cout << "terminate at:" << counter << std::endl;

		_cam->vk_detachFromAll(_dot);
		_cam->vk_detachFromAll(_line);
	}

private:
	float _angle;
	float _size;
	float _step;
	float _pointSize;
	float _lineWidth;
	float _alpha;
	bool _on;
	std::unique_ptr<VK4::Vk_Device> _device;
	std::unique_ptr<VK4::Vk_Viewer> _cam;
	std::shared_ptr<VK4::Vk_Dot<VK4::ObjectType_P_C>> _dot;
	std::shared_ptr<VK4::Vk_Line<VK4::ObjectType_P_C>> _line;
	std::shared_ptr<VK4::Vk_Line<VK4::ObjectType_P_C>> _coords;
	std::shared_ptr<VK4::Vk_Mesh<VK4::ObjectType_P_C>> _mesh;

	std::shared_ptr<VK4::Vk_Mesh<VK4::ObjectType_P_C>> _mesh2;
	std::shared_ptr<VK4::Vk_Line<VK4::ObjectType_P_C>> _mesh2Normals;

	void onoff(std::function<void()> repeat) {
		_on = !_on;
	}

	void incr_l_width(std::function<void()> repeat){
		_lineWidth++;
		_line->vk_updateLineWidth(_lineWidth);
		_cam->vk_rebuildAndRedraw();
	}

	void decr_l_width(std::function<void()> repeat){
		_lineWidth--;
		if(_lineWidth < 1.0f) _lineWidth = 1.0f;
		_line->vk_updateLineWidth(_lineWidth);
		_cam->vk_rebuildAndRedraw();
	}

	void incr_p_size(std::function<void()> repeat){
		_pointSize++;
		_dot->vk_updatePointSize(_pointSize);
		_cam->vk_rebuildAndRedraw();
	}

	void decr_p_size(std::function<void()> repeat){
		_pointSize--;
		if(_pointSize < 1.0f) _pointSize = 1.0f;
		_dot->vk_updatePointSize(_pointSize);
		_cam->vk_rebuildAndRedraw();
	}

	void decr_alpha(std::function<void()> repeat){
		_alpha -= 0.1f;
		if(_alpha < 0.0f) _alpha = 0.0f;
		_dot->vk_updateAlpha(_alpha);
		_cam->vk_rebuildAndRedraw();
	}

	void incr_alpha(std::function<void()> repeat){
		_alpha += 0.1f;
		if(_alpha > 1.0f) _alpha = 1.0f;
		_dot->vk_updateAlpha(_alpha);
		_cam->vk_rebuildAndRedraw();
	}

	void rotate(std::function<void()> repeat){
		_angle += 1.0f;
		if(_angle >= 360.0f){
			_angle = _angle - 360.0f;
		}

		_dot->vk_updatePoints(VK4::Vk_SampleObjects::Point_P(_angle / 180.0f * M_PI), 0, 0, VK4::Vk_ObjUpdate::Promptly);
		_cam->vk_rebuildAndRedraw();
		std::this_thread::sleep_for(std::chrono::microseconds(5000));
		
		if(_on) repeat();
	}

	void scale(std::function<void()> repeat) {
		if(_size >= 1.5f) _step = -0.01f;
		else if(_size <= 0.5f) _step = 0.01f;
		_size += _step;
		_dot->vk_updateModelMatrix(glm::tmat4x4<VK4::point_type>{_size,0,0,0, 0,_size,0,0, 0,0,_size,0, 1.5,1.5,0,1});
		_cam->vk_rebuildAndRedraw();
		std::this_thread::sleep_for(std::chrono::microseconds(5000));

		if(_on) repeat();
	}
};

int main(){
	Viewer v;
}
