
#include "./Defines.h"
#include "./Vk_ColorOp.hpp"
#include "./camera/Vk_GridLayout.hpp"
#include "./Vk_Viewer.hpp"
#include "./objects/dot/S_Dot_P_C.hpp"
#include "./objects/line/S_Line_P_C.hpp"
#include "./objects/mesh/S_Mesh_P_C.hpp"
#include "./lwws_win/include/lwws_key.hpp"

// #ifdef PYVK
namespace py = pybind11;
using namespace VK4;

PYBIND11_MODULE(_vkviewer, m) {
	m.doc() = "Vulkan Viewer for Numpy"; // optional module docstring

	py::enum_<LWWS::LWWS_Key::Special>(m, "lwws_key")
		.value("RandomKey", LWWS::LWWS_Key::Special::RandomKey)
		.value("Sleep", LWWS::LWWS_Key::Special::Sleep)  
		.value("F1", LWWS::LWWS_Key::Special::F1) 
		.value("F2", LWWS::LWWS_Key::Special::F2) 
		.value("F3", LWWS::LWWS_Key::Special::F3)
		.value("F4", LWWS::LWWS_Key::Special::F4)
		.value("F5", LWWS::LWWS_Key::Special::F5)
		.value("F6", LWWS::LWWS_Key::Special::F6)
		.value("F7", LWWS::LWWS_Key::Special::F7)
		.value("F8", LWWS::LWWS_Key::Special::F8)
		.value("F9", LWWS::LWWS_Key::Special::F9)
		.value("F10", LWWS::LWWS_Key::Special::F10)
		.value("F11", LWWS::LWWS_Key::Special::F11)
		.value("F12", LWWS::LWWS_Key::Special::F12)
		.value("NumLock", LWWS::LWWS_Key::Special::NumLock)        
		.value("LShift", LWWS::LWWS_Key::Special::LShift)         
		.value("RShift", LWWS::LWWS_Key::Special::RShift)         
		.value("LControl", LWWS::LWWS_Key::Special::LControl)       
		.value("RControl", LWWS::LWWS_Key::Special::RControl)
		.value("BackSpace", LWWS::LWWS_Key::Special::BackSpace) // 0x09
		.value("Up", LWWS::LWWS_Key::Special::Up)
		.value("Left", LWWS::LWWS_Key::Special::Left)
		.value("Down", LWWS::LWWS_Key::Special::Down)
		.value("Right", LWWS::LWWS_Key::Special::Right)
		.value("Escape", LWWS::LWWS_Key::Special::Escape)
		.value("Insert", LWWS::LWWS_Key::Special::Insert)
		.value("Home", LWWS::LWWS_Key::Special::Home)
		.value("End", LWWS::LWWS_Key::Special::End)
		.value("PageUp", LWWS::LWWS_Key::Special::PageUp)
		.value("PageDown", LWWS::LWWS_Key::Special::PageDown)
		.value("Delete", LWWS::LWWS_Key::Special::Delete)
		.value("AltGr", LWWS::LWWS_Key::Special::AltGr)
		.value("Alt", LWWS::LWWS_Key::Special::Alt)
		.value("Oem_1", LWWS::LWWS_Key::Special::Oem_1) // ü (CH), between L/P/0 and Backspace/Enter
		.value("Oem_2", LWWS::LWWS_Key::Special::Oem_2) // § (CH), right above Tab
		.value("Oem_3", LWWS::LWWS_Key::Special::Oem_3) // ¨ (CH), between L/P/0 and Backspace/Enter
		.value("Oem_4", LWWS::LWWS_Key::Special::Oem_4) // ' (CH), between L/P/0 and Backspace/Enter
		.value("Oem_5", LWWS::LWWS_Key::Special::Oem_5) // ä (CH), between L/P/0 and Backspace/Enter
		.value("Oem_6", LWWS::LWWS_Key::Special::Oem_6) // ^ (CH), between L/P/0 and Backspace/Enter
		.value("Oem_7", LWWS::LWWS_Key::Special::Oem_7) // ö (CH), between L/P/0 and Backspace/Enter
		.value("Oem_8", LWWS::LWWS_Key::Special::Oem_8) // $ (CH), between L/P/0 and Backspace/Enter
		.value("IntentionalSkip", LWWS::LWWS_Key::Special::IntentionalSkip); // this is just some placeholder to skip certain stuff

	py::enum_<Vk_DevicePreference>(m, "vk_device_preferences")
		.value("use_any_gpu", Vk_DevicePreference::USE_ANY_GPU)
		.value("use_integrated_gpu", Vk_DevicePreference::USE_INTEGRATED_GPU)
		.value("use_discrete_gpu", Vk_DevicePreference::USE_DISCRETE_GPU);

	py::enum_<Vk_ViewingType>(m, "vk_viewing_type")
		.value("local", Vk_ViewingType::LOCAL)
		.value("all", Vk_ViewingType::GLOBAL);

	py::enum_<Vk_CameraType>(m, "vk_camera_type")
		.value("Rasterizer_IM", Vk_CameraType::Rasterizer_IM);

	py::enum_<Vk_SteeringType>(m, "vk_steering_type")
		.value("camera_centric", Vk_SteeringType::CAMERA_CENTRIC)
		.value("object_centric", Vk_SteeringType::OBJECT_CENTRIC);

	py::enum_<RenderType>(m, "vk_render_type")
		.value("solid", RenderType::Solid)
		.value("wireframe", RenderType::Wireframe)
		.value("point", RenderType::Point);

	py::enum_<Topology>(m, "vk_topology")
		.value("points", Topology::Points)
		.value("lines", Topology::Lines)
		.value("triangles", Topology::Triangles);

	py::enum_<CullMode>(m, "vk_cull_mode")
		.value("none", CullMode::NoCulling)
		.value("back", CullMode::Back)
		.value("front", CullMode::Front);

	py::enum_<Vk_BufferSizeBehaviour>(m, "vk_buffer_characteristics")
		.value("init_empty_grow_1_5", Vk_BufferSizeBehaviour::Init_Empty_Grow_1_5)
		.value("init_empty_grow_2", Vk_BufferSizeBehaviour::Init_Empty_Grow_2)
		.value("init_1_0_grow_1_5", Vk_BufferSizeBehaviour::Init_1_0_Grow_1_5)
		.value("init_1_5_grow_1_5", Vk_BufferSizeBehaviour::Init_1_5_Grow_1_5)
		.value("init_1_0_grow_2", Vk_BufferSizeBehaviour::Init_1_0_Grow_2)
		.value("init_1_5_grow_2", Vk_BufferSizeBehaviour::Init_1_5_Grow_2);


	py::class_<Vk_RGBColor, std::shared_ptr<Vk_RGBColor>>(m, "vk_rgb_color")
		.def(py::init<float, float, float>(),
			py::arg("r"), py::arg("g"), py::arg("b"),
			py::call_guard<py::gil_scoped_release>())
		.def_readwrite("r", &Vk_RGBColor::r)
		.def_readwrite("g", &Vk_RGBColor::g)
		.def_readwrite("b", &Vk_RGBColor::b);

	py::class_<Vk_OklabColor, std::shared_ptr<Vk_OklabColor>>(m, "vk_oklab_color")
		.def(py::init<float, float, float>(),
			py::arg("L"), py::arg("a"), py::arg("b"),
			py::call_guard<py::gil_scoped_release>())
		.def_readwrite("L", &Vk_OklabColor::L)
		.def_readwrite("a", &Vk_OklabColor::a)
		.def_readwrite("b", &Vk_OklabColor::b);

	py::class_<Vk_ColorOp, std::shared_ptr<Vk_ColorOp>>(m, "vk_color_op")
		.def_static("rgb_to_oklab", 
					&Vk_ColorOp::rgb_to_oklab, 
					py::arg("rgb"), 
					"Convert vk_rgb_color to vk_oklab_color", 
					py::call_guard<py::gil_scoped_release>())
		.def_static("oklab_to_rgb", 
					&Vk_ColorOp::oklab_to_rgb, 
					py::arg("oklab"), 
					"Convert vk_oklab_color to vk_rgb_color", 
					py::call_guard<py::gil_scoped_release>())
		.def_static("oklab_lerp", 
					&Vk_ColorOp::oklab_lerp, 
					py::arg("p"), py::arg("from_color"), py::arg("to_color"), 
					"Linear interpolation between two oklab colors", 
					py::call_guard<py::gil_scoped_release>())
		.def_static("rgb_lerp", 
					&Vk_ColorOp::rgb_lerp, 
					py::arg("p"), py::arg("from_color"), py::arg("to"), 
					"Linear interpolation between two rgb colors. Interpolation first converts to Oklab then interpolates, then converts back.", 
					py::call_guard<py::gil_scoped_release>());


	py::class_<Vk_Viewport, std::shared_ptr<Vk_Viewport>>(m, "vk_viewport")
		.def(py::init<uint32_t, uint32_t, uint32_t, uint32_t, Vk_RGBColor>(),
			py::arg("x"), py::arg("y"), py::arg("width"), py::arg("height"), py::arg("clearColor"),
			py::call_guard<py::gil_scoped_release>())
		.def_readwrite("x", &Vk_Viewport::x)
		.def_readwrite("y", &Vk_Viewport::y)
		.def_readwrite("width", &Vk_Viewport::width)
		.def_readwrite("height", &Vk_Viewport::height)
		.def_readwrite("clearColor", &Vk_Viewport::clearColor);

	py::class_<Vk_CameraSpecs, std::shared_ptr<Vk_CameraSpecs>>(m, "vk_camera_specs")
		.def(py::init<
				Vk_CameraType,
				py::array_t<point_type, py::array::c_style>,
				py::array_t<point_type, py::array::c_style>,
				py::array_t<point_type, py::array::c_style>,
				point_type,
				point_type, 
				point_type,
				Vk_SteeringType
			>(),
			py::arg("type"), 
			py::arg("w_pos"), py::arg("w_look"), py::arg("w_up"), 
			py::arg("fow"), py::arg("w_near"), py::arg("w_far"), 
			py::arg("steering_type"),
			py::call_guard<py::gil_scoped_release>());
		// ... maybe not expose these becaue of the py::arrays above ...
		// .def_readwrite("type", &Vk_CameraSpecs::type)
		// .def_readwrite("w_pos", &Vk_CameraSpecs::wPos)
		// .def_readwrite("w_look", &Vk_CameraSpecs::wLook)
		// .def_readwrite("w_up", &Vk_CameraSpecs::wUp)
		// .def_readwrite("fow", &Vk_CameraSpecs::fow)
		// .def_readwrite("w_near", &Vk_CameraSpecs::wNear)
		// .def_readwrite("w_far", &Vk_CameraSpecs::wFar)
		// .def_readwrite("steeringType", &Vk_CameraSpecs::steeringType);

	py::class_<Vk_CameraCoords, std::shared_ptr<Vk_CameraCoords>>(m, "vk_camera_coords")
		.def(py::init())
		.def_readwrite("w_pos", &Vk_CameraCoords::wPos)
		.def_readwrite("w_look", &Vk_CameraCoords::wLook)
		.def_readwrite("w_up", &Vk_CameraCoords::wUp)
		.def_readwrite("x_axis", &Vk_CameraCoords::xAxis)
		.def_readwrite("y_axis", &Vk_CameraCoords::yAxis)
		.def_readwrite("z_axis", &Vk_CameraCoords::zAxis);

	py::class_<Vk_CameraInit, std::shared_ptr<Vk_CameraInit>>(m, "vk_camera_init")
		.def(py::init<int, int, int, Vk_Viewport, Vk_CameraSpecs>(),
			py::arg("cam_id"), py::arg("grid_x"), py::arg("grid_y"), py::arg("viewport"), py::arg("specs"),
			py::call_guard<py::gil_scoped_release>())
		.def_readwrite("cam_id", &Vk_CameraInit::viewportId)
		.def_readwrite("grid_x", &Vk_CameraInit::gridX)
		.def_readwrite("grid_y", &Vk_CameraInit::gridY)
		.def_readwrite("viewport", &Vk_CameraInit::viewport)
		.def_readwrite("specs", &Vk_CameraInit::specs);

	py::class_<Vk_ViewportMargins, std::shared_ptr<Vk_ViewportMargins>>(m, "vk_viewport_margins")
		.def(py::init<uint32_t, uint32_t, uint32_t, uint32_t>(),
			py::arg("left"), py::arg("right"), py::arg("top"), py::arg("bottom"),
			py::call_guard<py::gil_scoped_release>())
		.def_readwrite("left", &Vk_ViewportMargins::left)
		.def_readwrite("right", &Vk_ViewportMargins::right)
		.def_readwrite("top", &Vk_ViewportMargins::top)
		.def_readwrite("bottom", &Vk_ViewportMargins::bottom);

    py::class_<Vk_ViewerParams, std::shared_ptr<Vk_ViewerParams>>(m, "vk_viewer_params")
		.def(py::init<std::string, int, int, Vk_ViewingType, int, std::string>(),
			py::arg("name"),
			py::arg("width"), py::arg("height"), 
			py::arg("viewing_type"), 
			py::arg("fresh_pool_size")=100,
			py::arg("screenshot_save_path")="./",
			py::call_guard<py::gil_scoped_release>())
		.def_readwrite("width", &Vk_ViewerParams::width)
		.def_readwrite("height", &Vk_ViewerParams::height)
		.def_readwrite("freshPoolSize", &Vk_ViewerParams::freshPoolSize)
		.def_readwrite("viewingType", &Vk_ViewerParams::viewingType)
		.def_readwrite("screenshotSavePath", &Vk_ViewerParams::screenshotSavePath);


	// TODO: in the future export some more features here
	py::class_<Vk_Device, std::unique_ptr<Vk_Device, py::nodelete>>(m, "vk_device")
		.def(py::init<std::string, Vk_DevicePreference>(),
			 py::arg("name"),
			 py::arg("device_preferences")=Vk_DevicePreference::USE_ANY_GPU,
			 py::call_guard<py::gil_scoped_release>());


	py::class_<Vk_Renderable, std::shared_ptr<Vk_Renderable>>(m, "vk_renderable")
		.def(py::init(),
			py::call_guard<py::gil_scoped_release>())
		.def("vk_descriptor_count",
			 &Vk_Renderable::vk_descriptorCount,
			 py::call_guard<py::gil_scoped_release>())
		.def("vk_object_name",
			 &Vk_Renderable::vk_objectName,
			 py::call_guard<py::gil_scoped_release>())
		.def("vk_is_attached_to",
			 &Vk_Renderable::vk_isAttachedTo,
			 py::arg("cam_id"),
			 py::call_guard<py::gil_scoped_release>())
		.def("vk_update_model_matrix",
			 &Vk_Renderable::vk_updateModelMatrix,
			 py::arg("model_matrix"),
			 py::call_guard<py::gil_scoped_release>());

	py::class_<I_Object<ObjectType_P_C>, Vk_Renderable, std::shared_ptr<I_Object<ObjectType_P_C>>>(m, "i_object_p_c")
		.def(py::init(), py::call_guard<py::gil_scoped_release>());

	py::class_<Vk_Dot<ObjectType_P_C>, I_Object<ObjectType_P_C>, std::shared_ptr<Vk_Dot<ObjectType_P_C>>>(m, "vk_dot_p_c")
		.def(py::init(&S_Dot_P_C::create),
			py::arg("device"), py::arg("name"), 
			py::arg("model_matrix"),
			py::arg("points"), py::arg("colors"), py::arg("indices"),
			py::arg("point_size"), py::arg("alpha"),
			py::arg("cull_mode")=CullMode::NoCulling,
			py::arg("behaviour")=Vk_BufferSizeBehaviour::Init_1_0_Grow_1_5,
			"Create dot object with distinct buffers for points, colors and indices", 
			py::call_guard<py::gil_scoped_release>())
		.def("vk_update_points", 
			 &Vk_Dot<ObjectType_P_C>::vk_update_points, 
			 py::arg("points"), py::arg("new_from"),
			 py::call_guard<py::gil_scoped_release>())
		.def("vk_update_colors", 
			 &Vk_Dot<ObjectType_P_C>::vk_update_colors, 
			 py::arg("colors"), py::arg("new_from"),
			 py::call_guard<py::gil_scoped_release>())
		.def("vk_update_indices", 
			 &Vk_Dot<ObjectType_P_C>::vk_update_indices, 
			 py::arg("indices"), py::arg("new_from"),
			 py::call_guard<py::gil_scoped_release>())
		.def("vk_update_alpha", 
			 &Vk_Dot<ObjectType_P_C>::vk_updateAlpha, 
			 py::arg("alpha"),
			 py::call_guard<py::gil_scoped_release>())
		.def("vk_update_point_size", 
			 &Vk_Dot<ObjectType_P_C>::vk_updatePointSize, 
			 py::arg("point_size"),
			 py::call_guard<py::gil_scoped_release>());

	py::class_<Vk_Line<ObjectType_P_C>, I_Object<ObjectType_P_C>, std::shared_ptr<Vk_Line<ObjectType_P_C>>>(m, "vk_line_p_c")
		.def(py::init(&S_Line_P_C::create),
			py::arg("device"), py::arg("name"), 
			py::arg("model_matrix"),
			py::arg("points"), py::arg("colors"), py::arg("indices"),
			py::arg("line_width"), py::arg("alpha"),
			py::arg("cull_mode")=CullMode::NoCulling,
			py::arg("behaviour")=Vk_BufferSizeBehaviour::Init_1_0_Grow_1_5,
			"Create line object with distinct buffers for points, colors and indices", 
			py::call_guard<py::gil_scoped_release>())
		.def("vk_update_points", 
			 &Vk_Line<ObjectType_P_C>::vk_update_points, 
			 py::arg("points"), py::arg("new_from"),
			 py::call_guard<py::gil_scoped_release>())
		.def("vk_update_colors", 
			 &Vk_Line<ObjectType_P_C>::vk_update_colors, 
			 py::arg("colors"), py::arg("new_from"),
			 py::call_guard<py::gil_scoped_release>())
		.def("vk_update_indices", 
			 &Vk_Line<ObjectType_P_C>::vk_update_indices, 
			 py::arg("indices"), py::arg("new_from"),
			 py::call_guard<py::gil_scoped_release>())
		.def("vk_update_alpha", 
			 &Vk_Line<ObjectType_P_C>::vk_updateAlpha, 
			 py::arg("alpha"),
			 py::call_guard<py::gil_scoped_release>())
		.def("vk_update_line_width", 
			 &Vk_Line<ObjectType_P_C>::vk_updateLineWidth, 
			 py::arg("line_width"),
			 py::call_guard<py::gil_scoped_release>());

	py::class_<Vk_Mesh<ObjectType_P_C>, I_Object<ObjectType_P_C>, std::shared_ptr<Vk_Mesh<ObjectType_P_C>>>(m, "vk_mesh_p_c")
		.def(py::init(&S_Mesh_P_C::create),
			py::arg("device"), py::arg("name"), 
			py::arg("model_matrix"),
			py::arg("points"), py::arg("colors"), py::arg("indices"),
			py::arg("alpha"),
			py::arg("cull_mode")=CullMode::NoCulling,
			py::arg("render_type")=RenderType::Solid,
			py::arg("point_size"), py::arg("line_width"), 
			py::arg("behaviour")=Vk_BufferSizeBehaviour::Init_1_0_Grow_1_5,
			"Create mesh object with distinct buffers for points, colors and indices", 
			py::call_guard<py::gil_scoped_release>())
		.def("vk_update_points", 
			 &Vk_Mesh<ObjectType_P_C>::vk_update_points, 
			 py::arg("points"), py::arg("new_from"),
			 py::call_guard<py::gil_scoped_release>())
		.def("vk_update_colors", 
			 &Vk_Mesh<ObjectType_P_C>::vk_update_colors, 
			 py::arg("colors"), py::arg("new_from"),
			 py::call_guard<py::gil_scoped_release>())
		.def("vk_update_indices", 
			 &Vk_Mesh<ObjectType_P_C>::vk_update_indices, 
			 py::arg("indices"), py::arg("new_from"),
			 py::call_guard<py::gil_scoped_release>())
		.def("vk_update_alpha", 
			 &Vk_Mesh<ObjectType_P_C>::vk_updateAlpha, 
			 py::arg("alpha"),
			 py::call_guard<py::gil_scoped_release>())
		.def("vk_update_point_size", 
			 &Vk_Mesh<ObjectType_P_C>::vk_updatePointSize, 
			 py::arg("line_width"),
			 py::call_guard<py::gil_scoped_release>())
		.def("vk_update_line_width", 
			 &Vk_Mesh<ObjectType_P_C>::vk_updateLineWidth, 
			 py::arg("line_width"),
			 py::call_guard<py::gil_scoped_release>());


	py::class_<Vk_Viewer, std::shared_ptr<Vk_Viewer>>(m, "vk_viewer")
		.def(py::init<Vk_Device*, Vk_ViewerParams>(),
			 py::arg("device"),
			 py::arg("params"))
		.def("vk_get_version", 
			 &Vk_Viewer::vk_getVersion,
			 "Get version of viewer", 
			 py::call_guard<py::gil_scoped_release>())
		.def("vk_add_camera",
			 &Vk_Viewer::vk_addCameras,
			 py::arg("cameras"),
			 "Add camera to viewer",
			 py::call_guard<py::gil_scoped_release>())
		.def("vk_attach_to_all",
			 &Vk_Viewer::vk_attachToAll,
			 py::arg("object"),
			 "Attach object to all cameras",
			 py::call_guard<py::gil_scoped_release>())
		.def("vk_attach_to",
			 &Vk_Viewer::vk_attachTo,
			 py::arg("cam_id"), py::arg("object"),
			 "Attach object camera with cameraId cam_id",
			 py::call_guard<py::gil_scoped_release>())
		// .def("vk_build",
		// 	 &Vk_Viewer::vk_build,
		// 	 "Build renderer",
		// 	 py::call_guard<py::gil_scoped_release>())
		.def("vk_rebuild_and_redraw",
			 &Vk_Viewer::vk_rebuildAndRedraw,
			 "Rebuild renderer",
			 py::call_guard<py::gil_scoped_release>())
		.def("vk_detach_from_all",
			 &Vk_Viewer::vk_detachFromAll,
			 py::arg("object"),
			 "Detach object with objectName from all cameras",
			 py::call_guard<py::gil_scoped_release>())
		.def("vk_detach_from",
			 &Vk_Viewer::vk_detachFrom,
			 py::arg("cam_id"), py::arg("object"),
			 "Detach object with objectName from camera with cam_id",
			 py::call_guard<py::gil_scoped_release>())
		.def("vk_run_thread",
			 &Vk_Viewer::vk_runThread,
			 "Run camera loop in separate thread (use while viewer.vk_running(): ...)",
			 py::call_guard<py::gil_scoped_release>())
		.def("vk_run",
			 &Vk_Viewer::vk_run,
			 "Run camera loop",
			 py::call_guard<py::gil_scoped_release>())
		.def("vk_register_action",
			 &Vk_Viewer::vk_register_action,
			 py::arg("key"), py::arg("f"), py::arg("camera_id")=-1,
			 "Register action f to all cameras (Note: per-camera actions not yet supported)",
			 py::call_guard<py::gil_scoped_release>())
		.def("vk_unregister_action",
			 &Vk_Viewer::vk_unregister_action,
			 py::arg("key"),
			 "Unregister action bound to key",
			 py::call_guard<py::gil_scoped_release>())
		.def("vk_exec_action",
			 &Vk_Viewer::vk_exec_action,
			 py::arg("key"),
			 "Run action associated with the <key>. This is equivalent to pressing the associated <key> button.",
			 py::call_guard<py::gil_scoped_release>())
		.def("vk_camera_coords",
			 &Vk_Viewer::vk_cameraCoords,
			 py::return_value_policy::move,
			 py::arg("cam_id"),
			 "Get the camera specs of camera with id cam_id in the current state",
			 py::call_guard<py::gil_scoped_release>())
		.def("vk_running",
			 &Vk_Viewer::vk_running,
			 "True while the viewer is running, False afterwards (threadsafe)",
			 py::call_guard<py::gil_scoped_release>())
		.def("vk_pause_draw",
			 &Vk_Viewer::vk_pauseDraw,
			 "Stops updating the frame until vk_unpause_draw is called. For example, in order to update all objects if the user wants a synchronized picture.",
			 py::call_guard<py::gil_scoped_release>())
		.def("vk_unpause_draw",
			 &Vk_Viewer::vk_unpauseDraw,
			 "If vk_pause_draw was called previously, then resume drawing, otherwise, does nothing.",
			 py::call_guard<py::gil_scoped_release>());

		py::class_<I_LayoutPack, std::shared_ptr<I_LayoutPack>>(m, "vk_layout_pack")
		.def(py::init<Vk_CameraSpecs, Vk_RGBColor>(),
			py::arg("specs"), py::arg("clear_color"),
			py::call_guard<py::gil_scoped_release>());


	py::class_<Vk_GridLayout, std::shared_ptr<Vk_GridLayout>>(m, "vk_grid_layout")
		.def(py::init<int, int, int, int>(),
			py::arg("x_count"), py::arg("y_count"), py::arg("x_spacing"), py::arg("y_spacing"),
			py::call_guard<py::gil_scoped_release>())
		.def("vk_layout_list", 
			 &Vk_GridLayout::vk_layoutList, 
			 py::return_value_policy::reference,
			 py::arg("width"), py::arg("height"),
			 py::call_guard<py::gil_scoped_release>())
		.def("vk_count", 
			 &Vk_GridLayout::vk_count,
			 py::call_guard<py::gil_scoped_release>())
		.def("vk_add_camera", 
			 &Vk_GridLayout::vk_addCameras, 
			 py::arg("x"), py::arg("y"), py::arg("pack"), py::arg("override")=false,
			 py::call_guard<py::gil_scoped_release>());
}
// #else
// #include <iostream>
// int main(int argc, char** argv) {
// 	std::cout << "Pybind11 not available" << std::endl;
// }
// #endif