
#include "./Defines.h"

#include "./Vk_ColorOp.hpp"
#include "./camera/Vk_GridLayout.hpp"
#include "./Vk_Viewer.hpp"
#include "./objects/dot/S_Dot_P_C.hpp"
#include "./objects/line/S_Line_P_C.hpp"
#include "./objects/mesh/S_Mesh_P_C.hpp"
#include "./lwws_win/include/lwws_key.hpp"

#ifdef Bool
#undef Bool
#endif
#ifdef Complex
#undef Complex
#endif
#ifdef Float
#undef Float
#endif
#ifdef Int
#undef Int
#endif

#include <nanobind/nanobind.h>
#include <nanobind/stl/vector.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/array.h>
#include <nanobind/stl/tuple.h>
#include <nanobind/stl/set.h>
#include <nanobind/stl/map.h>
#include <nanobind/stl/shared_ptr.h>
#include <nanobind/ndarray.h>

// #ifdef PYVK
// namespace py = pybind11;
namespace nb = nanobind;
using namespace VK4;

class LWWS_Converter {
public:
	static bool lwwsStrKey2Int(const std::string& key, int& outKey){
		char c = static_cast<char>(*key.begin());
		int intKey = LWWS::LWWS_Key::KeyToInt(c);
		if(intKey < 0) {
			Vk_Logger::Error(typeid(NoneObj), "Unable to cast passed key");
			return false;
		}
		outKey = intKey;
		return True;
	}

	static bool lwwsSpecialKey2Int(const LWWS::LWWS_Key::Special& key, int& outKey){
		int intKey = LWWS::LWWS_Key::KeyToInt(key);
		if(intKey < 0) {
			Vk_Logger::Error(typeid(NoneObj), "Unable to cast passed key");
			return false;
		}
		outKey = intKey;
		return True;
	}
};

class Vk_NumpyTransformers {
public:
	static glm::tmat4x4<point_type> arrayToGLM4x4(const nb::ndarray<const point_type, nb::ndim<2>, nb::c_contig, nb::device::cpu>& inData){
		return glm::make_mat4x4(reinterpret_cast<const point_type*>(inData.data()));
	}

	static glm::tvec3<point_type> arrayToGLMv3(const nb::ndarray<const point_type, nb::ndim<1>, nb::c_contig, nb::device::cpu>& arr){
		return glm::make_vec3(reinterpret_cast<const point_type*>(arr.data()));
	}

	template<class T>
	static T* structArrayToCpp(const nb::ndarray<const point_type, nb::ndim<2>, nb::c_contig, nb::device::cpu>& inData, size_t& outLen){
		int innerDimensionLen = T::innerDimensionLen();
		size_t size = inData.size();
		if(size % innerDimensionLen != 0){
			Vk_Logger::RuntimeError(typeid(NoneObj), std::string(typeid(T).name()) + std::string(" array must be of size Nx") + std::to_string(innerDimensionLen) + std::string("!"));
		}
		outLen = static_cast<size_t>(size/innerDimensionLen);
		return reinterpret_cast<T*>(inData.data());
	}

	static const index_type* indexArrayToCpp(const nb::ndarray<const index_type, nb::c_contig, nb::device::cpu>& inData, size_t& outLen){
		return reinterpret_cast<const index_type*>(inData.data());
	}
};

NB_MODULE(_pyke, m) {
	m.doc() = "Vulkan Viewer for Numpy"; // optional module docstring

	nb::enum_<LWWS::LWWS_Key::Special>(m, "lwws_key", nb::is_flag())
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

	nb::enum_<Vk_DevicePreference>(m, "vk_device_preferences", nb::is_flag())
		.value("use_any_gpu", Vk_DevicePreference::USE_ANY_GPU)
		.value("use_integrated_gpu", Vk_DevicePreference::USE_INTEGRATED_GPU)
		.value("use_discrete_gpu", Vk_DevicePreference::USE_DISCRETE_GPU);

	nb::enum_<Vk_ViewingType>(m, "vk_viewing_type", nb::is_flag())
		.value("local", Vk_ViewingType::LOCAL)
		.value("all", Vk_ViewingType::GLOBAL);

	nb::enum_<Vk_CameraType>(m, "vk_camera_type", nb::is_flag())
		.value("Rasterizer_IM", Vk_CameraType::Rasterizer_IM);

	nb::enum_<Vk_SteeringType>(m, "vk_steering_type", nb::is_flag())
		.value("camera_centric", Vk_SteeringType::CAMERA_CENTRIC)
		.value("object_centric", Vk_SteeringType::OBJECT_CENTRIC);

	nb::enum_<RenderType>(m, "vk_render_type", nb::is_flag())
		.value("solid", RenderType::Solid)
		.value("wireframe", RenderType::Wireframe)
		.value("point", RenderType::Point);

	nb::enum_<Topology>(m, "vk_topology", nb::is_flag())
		.value("points", Topology::Points)
		.value("lines", Topology::Lines)
		.value("triangles", Topology::Triangles);

	nb::enum_<CullMode>(m, "vk_cull_mode", nb::is_flag())
		.value("none", CullMode::NoCulling)
		.value("back", CullMode::Back)
		.value("front", CullMode::Front);

	nb::enum_<Vk_BufferUpdateBehaviour>(m, "vk_buffer_update_behaviour", nb::is_flag())
		.value("global_lock", Vk_BufferUpdateBehaviour::GlobalLock)		 
		.value("double_buffering", Vk_BufferUpdateBehaviour::DoubleBuffering)
		.value("lazy_double_buffering", Vk_BufferUpdateBehaviour::LazyDoubleBuffering)
		.value("pinned", Vk_BufferUpdateBehaviour::Pinned);

	nb::enum_<Vk_BufferSizeBehaviour>(m, "vk_buffer_size_behaviour", nb::is_flag())
		.value("init_empty_grow_1_5", Vk_BufferSizeBehaviour::Init_Empty_Grow_1_5)
		.value("init_empty_grow_2", Vk_BufferSizeBehaviour::Init_Empty_Grow_2)
		.value("init_1_0_grow_1_5", Vk_BufferSizeBehaviour::Init_1_0_Grow_1_5)
		.value("init_1_5_grow_1_5", Vk_BufferSizeBehaviour::Init_1_5_Grow_1_5)
		.value("init_1_0_grow_2", Vk_BufferSizeBehaviour::Init_1_0_Grow_2)
		.value("init_1_5_grow_2", Vk_BufferSizeBehaviour::Init_1_5_Grow_2);


	nb::class_<Vk_RGBColor>(m, "vk_rgb_color")
		.def("__init__", [](Vk_RGBColor& self, float r, float g, float b){
				self = Vk_RGBColor{.r=r, .g=g, .b=b};
			},
			nb::arg("r"), nb::arg("g"), nb::arg("b"),
			nb::rv_policy::reference_internal,
			nb::call_guard<nb::gil_scoped_release>())
		.def_rw("r", &Vk_RGBColor::r)
		.def_rw("g", &Vk_RGBColor::g)
		.def_rw("b", &Vk_RGBColor::b);

	nb::class_<Vk_OklabColor>(m, "vk_oklab_color")
		.def("__init__", [](Vk_OklabColor& self, float L, float a, float b){
				self = Vk_OklabColor{.L=L, .a=a, .b=b};
			},
			nb::arg("L"), nb::arg("a"), nb::arg("b"),
			nb::rv_policy::reference_internal,
			nb::call_guard<nb::gil_scoped_release>())
		.def_rw("L", &Vk_OklabColor::L)
		.def_rw("a", &Vk_OklabColor::a)
		.def_rw("b", &Vk_OklabColor::b);

	nb::class_<Vk_ColorOp>(m, "vk_color_op")
		.def_static("rgb_to_oklab", 
					&Vk_ColorOp::rgb_to_oklab, 
					nb::arg("rgb"), 
					"Convert vk_rgb_color to vk_oklab_color", 
					nb::rv_policy::reference_internal,
					nb::call_guard<nb::gil_scoped_release>())
		.def_static("oklab_to_rgb", 
					&Vk_ColorOp::oklab_to_rgb, 
					nb::arg("oklab"), 
					"Convert vk_oklab_color to vk_rgb_color", 
					nb::rv_policy::reference_internal,
					nb::call_guard<nb::gil_scoped_release>())
		.def_static("oklab_lerp", 
					&Vk_ColorOp::oklab_lerp, 
					nb::arg("p"), nb::arg("from_color"), nb::arg("to_color"), 
					"Linear interpolation between two oklab colors", 
					nb::rv_policy::reference_internal,
					nb::call_guard<nb::gil_scoped_release>())
		.def_static("rgb_lerp", 
					&Vk_ColorOp::rgb_lerp, 
					nb::arg("p"), nb::arg("from_color"), nb::arg("to"), 
					"Linear interpolation between two rgb colors. Interpolation first converts to Oklab then interpolates, then converts back.", 
					nb::rv_policy::reference_internal,
					nb::call_guard<nb::gil_scoped_release>());


	nb::class_<Vk_Viewport>(m, "vk_viewport")
		.def("__init__", [](Vk_Viewport& self, int32_t x, int32_t y, uint32_t width, uint32_t height, std::shared_ptr<Vk_RGBColor> clearColor){
				self = Vk_Viewport{.x=x,.y=y,.width=width,.height=height,.clearColor=*clearColor.get()};
			},
			nb::arg("x"), nb::arg("y"), nb::arg("width"), nb::arg("height"), nb::arg("clearColor"),
			nb::rv_policy::reference_internal,
			nb::call_guard<nb::gil_scoped_release>())
		.def_rw("x", &Vk_Viewport::x)
		.def_rw("y", &Vk_Viewport::y)
		.def_rw("width", &Vk_Viewport::width)
		.def_rw("height", &Vk_Viewport::height)
		.def_rw("clearColor", &Vk_Viewport::clearColor);

	// NOTE: needs revision...
	nb::class_<Vk_CameraSpecs>(m, "vk_camera_specs")
		.def("__init__", [](
			Vk_CameraSpecs& self,
			const Vk_CameraType& type,
			const nb::ndarray<const point_type, nb::ndim<1>, nb::c_contig, nb::device::cpu>& w_pos,
			const nb::ndarray<const point_type, nb::ndim<1>, nb::c_contig, nb::device::cpu>& w_look,
			const nb::ndarray<const point_type, nb::ndim<1>, nb::c_contig, nb::device::cpu>& w_up,
			point_type fow,
			point_type w_near,
			point_type w_far,
			Vk_SteeringType steering_type
		){
			self = Vk_CameraSpecs{
				.type=type,
				.wPos = Vk_NumpyTransformers::arrayToGLMv3(w_pos),
				.wLook = Vk_NumpyTransformers::arrayToGLMv3(w_look),
				.wUp = Vk_NumpyTransformers::arrayToGLMv3(w_up),
				.fow = fow,
				.wNear = w_near,
				.wFar = w_far,
				.steeringType = steering_type
			};
		},
		nb::arg("type"), 
		nb::arg("w_pos"), nb::arg("w_look"), nb::arg("w_up"), 
		nb::arg("fow"), nb::arg("w_near"), nb::arg("w_far"), 
		nb::arg("steering_type"),
		nb::rv_policy::reference_internal,
		nb::call_guard<nb::gil_scoped_release>());

	nb::class_<Vk_CameraCoords>(m, "vk_camera_coords")
		.def(nb::init())
		.def_rw("w_pos", &Vk_CameraCoords::wPos, nb::rv_policy::copy, nb::call_guard<nb::gil_scoped_release>())
		.def_rw("w_look", &Vk_CameraCoords::wLook, nb::rv_policy::copy, nb::call_guard<nb::gil_scoped_release>())
		.def_rw("w_up", &Vk_CameraCoords::wUp, nb::rv_policy::copy, nb::call_guard<nb::gil_scoped_release>())
		.def_rw("x_axis", &Vk_CameraCoords::xAxis, nb::rv_policy::copy, nb::call_guard<nb::gil_scoped_release>())
		.def_rw("y_axis", &Vk_CameraCoords::yAxis, nb::rv_policy::copy, nb::call_guard<nb::gil_scoped_release>())
		.def_rw("z_axis", &Vk_CameraCoords::zAxis, nb::rv_policy::copy, nb::call_guard<nb::gil_scoped_release>());

	nb::class_<Vk_CameraInit>(m, "vk_camera_init")
		.def("__init__", [](
			Vk_CameraInit& self,
			int camId, int gridX, int gridY, 
			std::shared_ptr<Vk_Viewport> viewport, 
			std::shared_ptr<Vk_CameraSpecs> specs){
				self = Vk_CameraInit{.camId=camId, .gridX=gridX, .gridY=gridY, .viewport=*viewport.get(), .specs=*specs.get()};
			},
			nb::arg("cam_id"), nb::arg("grid_x"), nb::arg("grid_y"), nb::arg("viewport"), nb::arg("specs"),
			nb::rv_policy::reference_internal,
			nb::call_guard<nb::gil_scoped_release>())
		.def_rw("cam_id", &Vk_CameraInit::camId, nb::call_guard<nb::gil_scoped_release>())
		.def_rw("grid_x", &Vk_CameraInit::gridX, nb::call_guard<nb::gil_scoped_release>())
		.def_rw("grid_y", &Vk_CameraInit::gridY, nb::call_guard<nb::gil_scoped_release>())
		.def_rw("viewport", &Vk_CameraInit::viewport, nb::call_guard<nb::gil_scoped_release>())
		.def_rw("specs", &Vk_CameraInit::specs, nb::call_guard<nb::gil_scoped_release>());

	nb::class_<Vk_ViewportMargins>(m, "vk_viewport_margins")
		.def("__init__", [](
			Vk_ViewportMargins& self,
			int32_t left, int32_t right, int32_t top, int32_t bottom){
				self = Vk_ViewportMargins{.left=left, .right=right, .top=top, .bottom=bottom};
			},
			nb::arg("left"), nb::arg("right"), nb::arg("top"), nb::arg("bottom"),
			nb::rv_policy::reference_internal,
			nb::call_guard<nb::gil_scoped_release>())
		.def_rw("left", &Vk_ViewportMargins::left)
		.def_rw("right", &Vk_ViewportMargins::right)
		.def_rw("top", &Vk_ViewportMargins::top)
		.def_rw("bottom", &Vk_ViewportMargins::bottom);

    nb::class_<Vk_ViewerParams>(m, "vk_viewer_params")
		.def(nb::new_([](
			const std::string& name, int width, int height, 
			Vk_ViewingType viewingType, 
			int freshPoolSize, 
			const std::string& screenshotSavePath){
				return std::make_shared<Vk_ViewerParams>(Vk_ViewerParams{
					.name=name, 
					.width=width, 
					.height=height, 
					.freshPoolSize=freshPoolSize, 
					.viewingType=viewingType, 
					.screenshotSavePath=screenshotSavePath
				});
			}),
			nb::arg("name"),
			nb::arg("width"), nb::arg("height"), 
			nb::arg("viewing_type"), 
			nb::arg("fresh_pool_size")=100,
			nb::arg("screenshot_save_path")="./",
			nb::call_guard<nb::gil_scoped_release>())
		.def_rw("width", &Vk_ViewerParams::width)
		.def_rw("height", &Vk_ViewerParams::height)
		.def_rw("freshPoolSize", &Vk_ViewerParams::freshPoolSize)
		.def_rw("viewingType", &Vk_ViewerParams::viewingType)
		.def_rw("screenshotSavePath", &Vk_ViewerParams::screenshotSavePath);


	// TODO: in the future export some more features here
	nb::class_<Vk_Device>(m, "vk_device")
		.def(nb::new_([](std::string name, const Vk_DevicePreference& prefs){
				return std::make_shared<Vk_Device>(name, prefs);
			}),
			nb::arg("name"),
			nb::arg("device_preferences")=Vk_DevicePreference::USE_ANY_GPU,
			nb::rv_policy::reference_internal,
			nb::call_guard<nb::gil_scoped_release>());

	nb::class_<Vk_Renderable>(m, "vk_renderable")
		.def(nb::init(),
			nb::rv_policy::reference_internal,
			nb::call_guard<nb::gil_scoped_release>())
		.def("vk_descriptor_count",
			 &Vk_Renderable::vk_descriptorCount,
			 nb::call_guard<nb::gil_scoped_release>())
		.def("vk_object_name",
			 &Vk_Renderable::vk_objectName,
			 nb::call_guard<nb::gil_scoped_release>())
		.def("vk_is_attached_to",
			 &Vk_Renderable::vk_isAttachedTo,
			 nb::arg("cam_id"),
			 nb::call_guard<nb::gil_scoped_release>())
		.def("vk_update_model_matrix",
			 &Vk_Renderable::vk_updateModelMatrix,
			 nb::arg("model_matrix"),
			 nb::call_guard<nb::gil_scoped_release>());

	nb::class_<I_Object<ObjectType_P_C>, Vk_Renderable>(m, "i_object_p_c")
		.def(nb::init(),
			nb::rv_policy::reference_internal,
			nb::call_guard<nb::gil_scoped_release>());

	nb::class_<Vk_Dot<ObjectType_P_C>, I_Object<ObjectType_P_C>>(m, "vk_dot_p_c")
		.def(nb::new_([](
				std::shared_ptr<Vk_Device> device,
				std::string name,
				const nb::ndarray<const point_type, nb::c_contig, nb::device::cpu>& modelMatrix,
				const nb::ndarray<const point_type, nb::c_contig, nb::device::cpu>& points,
				const nb::ndarray<const point_type, nb::c_contig, nb::device::cpu>& colors,
				const nb::ndarray<const index_type, nb::c_contig, nb::device::cpu>& indices,
				float pointSize,
				float alpha,
				// Topology topology = VK4::Topology::Points,
				CullMode cullMode = VK4::CullMode::NoCulling,
				// RenderType renderType = VK4::RenderType::Point,
				Vk_BufferUpdateBehaviour updateBehaviour = Vk_BufferUpdateBehaviour::GlobalLock,
				Vk_BufferSizeBehaviour sizeBehaviour = Vk_BufferSizeBehaviour::Init_1_0_Grow_1_5
			){
				return S_Dot_P_C::create(
					device.get(),
					name,
					std::span<const point_type>(reinterpret_cast<const point_type*>(modelMatrix.data()), modelMatrix.size()), 
					std::span<const point_type>(reinterpret_cast<const point_type*>(points.data()), points.size()),
					std::span<const point_type>(reinterpret_cast<const point_type*>(colors.data()), colors.size()),
					std::span<const index_type>(reinterpret_cast<const index_type*>(indices.data()), indices.size()),
					pointSize, alpha,
					cullMode,
					updateBehaviour,
					sizeBehaviour
				);
			}),
			nb::arg("device"), nb::arg("name"), 
			nb::arg("model_matrix"),
			nb::arg("points"), nb::arg("colors"), nb::arg("indices"),
			nb::arg("point_size"), nb::arg("alpha"),
			nb::arg("cull_mode")=CullMode::NoCulling,
			nb::arg("updateBehaviour")=Vk_BufferUpdateBehaviour::GlobalLock,
			nb::arg("sizeBehaviour")=Vk_BufferSizeBehaviour::Init_1_0_Grow_1_5,
			"Create dot object with distinct buffers for points, colors and indices", 
			nb::call_guard<nb::gil_scoped_release>())
		.def("vk_update_points", 
			 [](
				std::shared_ptr<Vk_Dot<ObjectType_P_C>> self,
				const nb::ndarray<const point_type, nb::c_contig, nb::device::cpu>& points,
				size_t new_from
			 ){
				self->vk_updatePoints(std::span<const point_type>(reinterpret_cast<const point_type*>(points.data()), points.size()), new_from);
			 }, 
			 nb::arg("points"), nb::arg("new_from"),
			 nb::call_guard<nb::gil_scoped_release>())
		.def("vk_update_colors", 
			 [](
				std::shared_ptr<Vk_Dot<ObjectType_P_C>> self,
				const nb::ndarray<const point_type, nb::c_contig, nb::device::cpu>& colors,
				size_t new_from
			 ){
				self->vk_updateColors(std::span<const point_type>(reinterpret_cast<const point_type*>(colors.data()), colors.size()), new_from);
			 }, 
			 nb::arg("colors"), nb::arg("new_from"),
			 nb::call_guard<nb::gil_scoped_release>())
		.def("vk_update_indices", 
			 [](
				std::shared_ptr<Vk_Dot<ObjectType_P_C>> self,
				const nb::ndarray<const index_type, nb::c_contig, nb::device::cpu>& indices,
				size_t new_from
			 ){
				self->vk_updateIndices(std::span<const index_type>(reinterpret_cast<const index_type*>(indices.data()), indices.size()), new_from);
			 }, 
			 nb::arg("indices"), nb::arg("new_from"),
			 nb::call_guard<nb::gil_scoped_release>())
		.def("vk_update_alpha", 
			 &Vk_Dot<ObjectType_P_C>::vk_updateAlpha, 
			 nb::arg("alpha"),
			 nb::call_guard<nb::gil_scoped_release>())
		.def("vk_update_point_size", 
			 &Vk_Dot<ObjectType_P_C>::vk_updatePointSize, 
			 nb::arg("point_size"),
			 nb::call_guard<nb::gil_scoped_release>());

	nb::class_<Vk_Line<ObjectType_P_C>, I_Object<ObjectType_P_C>>(m, "vk_line_p_c")
		.def(nb::new_([](
				std::shared_ptr<Vk_Device> device,
				std::string name,
				const nb::ndarray<const point_type, nb::c_contig, nb::device::cpu>& modelMatrix,
				const nb::ndarray<const point_type, nb::c_contig, nb::device::cpu>& points,
				const nb::ndarray<const point_type, nb::c_contig, nb::device::cpu>& colors,
				const nb::ndarray<const index_type, nb::c_contig, nb::device::cpu>& indices,
				float lineWidth,
				float alpha,
				// Topology topology = VK4::Topology::Points,
				CullMode cullMode = VK4::CullMode::NoCulling,
				// RenderType renderType = VK4::RenderType::Point,
				Vk_BufferUpdateBehaviour updateBehaviour = Vk_BufferUpdateBehaviour::GlobalLock,
				Vk_BufferSizeBehaviour sizeBehaviour = Vk_BufferSizeBehaviour::Init_1_0_Grow_1_5
			){
				return S_Line_P_C::create(
					device.get(),
					name,
					std::span<const point_type>(reinterpret_cast<const point_type*>(modelMatrix.data()), modelMatrix.size()), 
					std::span<const point_type>(reinterpret_cast<const point_type*>(points.data()), points.size()),
					std::span<const point_type>(reinterpret_cast<const point_type*>(colors.data()), colors.size()),
					std::span<const index_type>(reinterpret_cast<const index_type*>(indices.data()), indices.size()),
					lineWidth, alpha,
					cullMode,
					updateBehaviour,
					sizeBehaviour
				);
			}),
			nb::arg("device"), nb::arg("name"), 
			nb::arg("model_matrix"),
			nb::arg("points"), nb::arg("colors"), nb::arg("indices"),
			nb::arg("line_width"), nb::arg("alpha"),
			nb::arg("cull_mode")=CullMode::NoCulling,
			nb::arg("updateBehaviour")=Vk_BufferUpdateBehaviour::GlobalLock,
			nb::arg("sizeBehaviour")=Vk_BufferSizeBehaviour::Init_1_0_Grow_1_5,
			"Create line object with distinct buffers for points, colors and indices", 
			nb::call_guard<nb::gil_scoped_release>())
		.def("vk_update_points", 
			[](
				std::shared_ptr<Vk_Line<ObjectType_P_C>> self,
				const nb::ndarray<const point_type, nb::c_contig, nb::device::cpu>& points,
				size_t new_from
			){
				self->vk_updatePoints(std::span<const point_type>(reinterpret_cast<const point_type*>(points.data()), points.size()), new_from);
			}, 
			nb::arg("points"), nb::arg("new_from"),
			nb::call_guard<nb::gil_scoped_release>())
		.def("vk_update_colors", 
			[](
				std::shared_ptr<Vk_Line<ObjectType_P_C>> self,
				const nb::ndarray<const point_type, nb::c_contig, nb::device::cpu>& colors,
				size_t new_from
			){
				self->vk_updateColors(std::span<const point_type>(reinterpret_cast<const point_type*>(colors.data()), colors.size()), new_from);
			}, 
			nb::arg("colors"), nb::arg("new_from"),
			nb::call_guard<nb::gil_scoped_release>())
		.def("vk_update_indices", 
			[](
				std::shared_ptr<Vk_Line<ObjectType_P_C>> self,
				const nb::ndarray<const index_type, nb::c_contig, nb::device::cpu>& indices,
				size_t new_from
			){
				self->vk_updateIndices(std::span<const index_type>(reinterpret_cast<const index_type*>(indices.data()), indices.size()), new_from);
			}, 
			nb::arg("indices"), nb::arg("new_from"),
			nb::call_guard<nb::gil_scoped_release>())
		.def("vk_update_alpha", 
			 &Vk_Line<ObjectType_P_C>::vk_updateAlpha, 
			 nb::arg("alpha"),
			 nb::call_guard<nb::gil_scoped_release>())
		.def("vk_update_line_width", 
			 &Vk_Line<ObjectType_P_C>::vk_updateLineWidth, 
			 nb::arg("line_width"),
			 nb::call_guard<nb::gil_scoped_release>());

	nb::class_<Vk_Mesh<ObjectType_P_C>, I_Object<ObjectType_P_C>>(m, "vk_mesh_p_c")
		.def(nb::new_([](
				std::shared_ptr<Vk_Device> device,
				std::string name,
				const nb::ndarray<const point_type, nb::c_contig, nb::device::cpu>& modelMatrix,
				const nb::ndarray<const point_type, nb::c_contig, nb::device::cpu>& points,
				const nb::ndarray<const point_type, nb::c_contig, nb::device::cpu>& colors,
				const nb::ndarray<const index_type, nb::c_contig, nb::device::cpu>& indices,
				// Topology topology = VK4::Topology::Points,
				float alpha=1.0f,
				CullMode cullMode = VK4::CullMode::Back,
				RenderType renderType = VK4::RenderType::Solid,
				float pointSize=1.0f,
				float lineWidth=1.0f,
				Vk_BufferUpdateBehaviour sizeBehaviour = Vk_BufferUpdateBehaviour::GlobalLock,
				Vk_BufferSizeBehaviour updateBehaviour = Vk_BufferSizeBehaviour::Init_1_0_Grow_1_5
			){
				return S_Mesh_P_C::create(
					device.get(),
					name,
					std::span<const point_type>(reinterpret_cast<const point_type*>(modelMatrix.data()), modelMatrix.size()), 
					std::span<const point_type>(reinterpret_cast<const point_type*>(points.data()), points.size()),
					std::span<const point_type>(reinterpret_cast<const point_type*>(colors.data()), colors.size()),
					std::span<const index_type>(reinterpret_cast<const index_type*>(indices.data()), indices.size()),
					alpha,
					cullMode,
					renderType,
					pointSize,
					lineWidth,
					sizeBehaviour,
					updateBehaviour
				);
			}),
			nb::arg("device"), 
			nb::arg("name"), 
			nb::arg("model_matrix"),
			nb::arg("points"), nb::arg("colors"), nb::arg("indices"),
			nb::arg("alpha"),
			nb::arg("cull_mode")=CullMode::NoCulling,
			nb::arg("render_type")=RenderType::Solid,
			nb::arg("point_size"), nb::arg("line_width"), 
			nb::arg("updateBehaviour")=Vk_BufferUpdateBehaviour::GlobalLock,
			nb::arg("sizeBehaviour")=Vk_BufferSizeBehaviour::Init_1_0_Grow_1_5,
			"Create mesh object with distinct buffers for points, colors and indices", 
			nb::call_guard<nb::gil_scoped_release>())
		.def("vk_update_points", 
			[](
				std::shared_ptr<Vk_Mesh<ObjectType_P_C>> self,
				const nb::ndarray<const point_type, nb::c_contig, nb::device::cpu>& points,
				size_t new_from
			){
				self->vk_updatePoints(std::span<const point_type>(reinterpret_cast<const point_type*>(points.data()), points.size()), new_from);
			}, 
			nb::arg("points"), nb::arg("new_from"),
			nb::call_guard<nb::gil_scoped_release>())
		.def("vk_update_colors", 
			[](
				std::shared_ptr<Vk_Mesh<ObjectType_P_C>> self,
				const nb::ndarray<const point_type, nb::c_contig, nb::device::cpu>& colors,
				size_t new_from
			){
				self->vk_updateColors(std::span<const point_type>(reinterpret_cast<const point_type*>(colors.data()), colors.size()), new_from);
			}, 
			nb::arg("colors"), nb::arg("new_from"),
			nb::call_guard<nb::gil_scoped_release>())
		.def("vk_update_indices", 
			[](
				std::shared_ptr<Vk_Mesh<ObjectType_P_C>> self,
				const nb::ndarray<const index_type, nb::c_contig, nb::device::cpu>& indices,
				size_t new_from
			){
				self->vk_updateIndices(std::span<const index_type>(reinterpret_cast<const index_type*>(indices.data()), indices.size()), new_from);
			}, 
			nb::arg("indices"), nb::arg("new_from"),
			nb::call_guard<nb::gil_scoped_release>())
		.def("vk_update_alpha", 
			 &Vk_Mesh<ObjectType_P_C>::vk_updateAlpha, 
			 nb::arg("alpha"),
			 nb::call_guard<nb::gil_scoped_release>())
		.def("vk_update_point_size", 
			 &Vk_Mesh<ObjectType_P_C>::vk_updatePointSize, 
			 nb::arg("line_width"),
			 nb::call_guard<nb::gil_scoped_release>())
		.def("vk_update_line_width", 
			 &Vk_Mesh<ObjectType_P_C>::vk_updateLineWidth, 
			 nb::arg("line_width"),
			 nb::call_guard<nb::gil_scoped_release>());

	nb::class_<Vk_Viewer>(m, "vk_viewer")
		.def(nb::new_([](std::shared_ptr<Vk_Device> device, const Vk_ViewerParams& params){
				return std::make_shared<Vk_Viewer>(device.get(), params);
			}),
			nb::arg("device"),
			nb::arg("params"),
			nb::call_guard<nb::gil_scoped_release>())
		.def("vk_get_version", 
			&Vk_Viewer::vk_getVersion,
			"Get version of viewer", 
			nb::call_guard<nb::gil_scoped_release>())
		.def("vk_add_camera",
			 &Vk_Viewer::vk_addCamera,
			 nb::arg("cameras"),
			 "Add camera to viewer",
			 nb::call_guard<nb::gil_scoped_release>())
		.def("vk_attach_to_all",
			 &Vk_Viewer::vk_attachToAll,
			 nb::arg("object"),
			 "Attach object to all cameras",
			 nb::call_guard<nb::gil_scoped_release>())
		.def("vk_attach_to",
			 &Vk_Viewer::vk_attachTo,
			 nb::arg("cam_id"), nb::arg("object"),
			 "Attach object camera with cameraId cam_id",
			 nb::call_guard<nb::gil_scoped_release>())
		// .def("vk_build",
		// 	 &Vk_Viewer::vk_build,
		// 	 "Build renderer",
		// 	 nb::call_guard<nb::gil_scoped_release>())
		.def("vk_rebuild_and_redraw",
			 &Vk_Viewer::vk_rebuildAndRedraw,
			 "Rebuild renderer",
			 nb::call_guard<nb::gil_scoped_release>())
		.def("vk_detach_from_all",
			 &Vk_Viewer::vk_detachFromAll,
			 nb::arg("object"),
			 "Detach object with objectName from all cameras",
			 nb::call_guard<nb::gil_scoped_release>())
		.def("vk_detach_from",
			 &Vk_Viewer::vk_detachFrom,
			 nb::arg("cam_id"), nb::arg("object"),
			 "Detach object with objectName from camera with cam_id",
			 nb::call_guard<nb::gil_scoped_release>())
		.def("vk_run_thread",
			 &Vk_Viewer::vk_runThread,
			 "Run camera loop in separate thread (use while viewer.vk_running(): ...)",
			 nb::call_guard<nb::gil_scoped_release>())
		.def("vk_run",
			 &Vk_Viewer::vk_run,
			 "Run camera loop",
			 nb::call_guard<nb::gil_scoped_release>())
		.def("vk_register_action",
			 [](std::shared_ptr<Vk_Viewer> self, nb::object obj, const std::string& key, nb::callable f, int cameraId=-1){
				if(cameraId >= 0){
					Vk_Logger::RuntimeError(typeid(self), "Per camera localized actions not supported yet!");
					return false;
				}
				int intKey;
				if(!LWWS_Converter::lwwsStrKey2Int(key, intKey)) return false;
				auto pyFunc = Vk_PyFunc(obj, f);
				return self->vk_registerAction(intKey, &pyFunc, cameraId);
				return true;
			 },
			 nb::arg("obj"), nb::arg("key"), nb::arg("f"), nb::arg("camera_id")=-1,
			 "Register action f to all cameras (Note: per-camera actions not yet supported)",
			 nb::call_guard<nb::gil_scoped_release>())
		.def("vk_register_action",
			[](std::shared_ptr<Vk_Viewer> self, nb::object obj, const LWWS::LWWS_Key::Special& key, nb::callable f, int cameraId=-1){
				if(cameraId >= 0){
					Vk_Logger::RuntimeError(typeid(self), "Per camera localized actions not supported yet!");
					return false;
				}
				int intKey;
				if(!LWWS_Converter::lwwsSpecialKey2Int(key, intKey)) return false;
				auto pyFunc = Vk_PyFunc(obj, f);
				return self->vk_registerAction(intKey, &pyFunc, cameraId);
				return true;
			},
			nb::arg("obj"), nb::arg("key"), nb::arg("f"), nb::arg("camera_id")=-1,
			"Register action f to all cameras (Note: per-camera actions not yet supported)",
			nb::call_guard<nb::gil_scoped_release>())
		.def("vk_unregister_action",
			 [](std::shared_ptr<Vk_Viewer> self, const std::string& key){
				char c = static_cast<char>(*key.begin());
				int intKey;
				if(!LWWS_Converter::lwwsStrKey2Int(key, intKey)) return false;
				return self->vk_unregisterAction(intKey);
			 },
			 nb::arg("key"),
			 "Unregister action bound to key",
			 nb::call_guard<nb::gil_scoped_release>())
		.def("vk_unregister_action",
			[](std::shared_ptr<Vk_Viewer> self, const LWWS::LWWS_Key::Special& key){
				int intKey;
				if(!LWWS_Converter::lwwsSpecialKey2Int(key, intKey)) return false;
				return self->vk_unregisterAction(intKey);
			},
			nb::arg("key"),
			"Unregister action bound to key",
			nb::call_guard<nb::gil_scoped_release>())
		.def("vk_exec_action",
			 [](std::shared_ptr<Vk_Viewer> self, const std::string& key){
				int intKey;
				if(!LWWS_Converter::lwwsStrKey2Int(key, intKey)) return;
				return self->vk_execAction(intKey);
			 },
			 nb::arg("key"),
			 "Run action associated with the <key>. This is equivalent to pressing the associated <key> button.",
			 nb::call_guard<nb::gil_scoped_release>())
		.def("vk_exec_action",
			[](std::shared_ptr<Vk_Viewer> self, const LWWS::LWWS_Key::Special& key){
				int intKey;
				if(!LWWS_Converter::lwwsSpecialKey2Int(key, intKey)) return;
				return self->vk_execAction(intKey);
			},
			nb::arg("key"),
			"Run action associated with the <key>. This is equivalent to pressing the associated <key> button.",
			nb::call_guard<nb::gil_scoped_release>())
		.def("vk_camera_coords",
			 &Vk_Viewer::vk_cameraCoords,
			 nb::rv_policy::move,
			 nb::arg("cam_id"),
			 "Get the camera specs of camera with id cam_id in the current state",
			 nb::call_guard<nb::gil_scoped_release>())
		.def("vk_running",
			 &Vk_Viewer::vk_running,
			 "True while the viewer is running, False afterwards (threadsafe)",
			 nb::call_guard<nb::gil_scoped_release>())
		.def("vk_pause_draw",
			 &Vk_Viewer::vk_pauseDraw,
			 "Stops updating the frame until vk_unpause_draw is called. For example, in order to update all objects if the user wants a synchronized picture.",
			 nb::call_guard<nb::gil_scoped_release>())
		.def("vk_unpause_draw",
			 &Vk_Viewer::vk_unpauseDraw,
			 "If vk_pause_draw was called previously, then resume drawing, otherwise, does nothing.",
			 nb::call_guard<nb::gil_scoped_release>());

	nb::class_<I_LayoutPack>(m, "vk_layout_pack")
		.def("__init__", [](
				I_LayoutPack& self,
				const Vk_CameraSpecs& specs, 
				const Vk_RGBColor& clear_color
			){
				self = I_LayoutPack{.specs=specs, .clearColor=clear_color};
			},
			nb::arg("specs"), nb::arg("clear_color"),
			nb::call_guard<nb::gil_scoped_release>());

	nb::class_<Vk_GridLayout>(m, "vk_grid_layout")
		.def(nb::init<int, int, int, int>(),
			nb::arg("x_count"), nb::arg("y_count"), nb::arg("x_spacing"), nb::arg("y_spacing"),
			nb::call_guard<nb::gil_scoped_release>())
		.def("vk_layout_list", 
			 &Vk_GridLayout::vk_layoutList, 
			 nb::rv_policy::reference,
			 nb::arg("width"), nb::arg("height"),
			 nb::call_guard<nb::gil_scoped_release>())
		.def("vk_count", 
			 &Vk_GridLayout::vk_count,
			 nb::call_guard<nb::gil_scoped_release>())
		.def("vk_add_camera", 
			 &Vk_GridLayout::vk_addCamera, 
			 nb::arg("x"), nb::arg("y"), nb::arg("pack"), nb::arg("override")=false,
			 nb::call_guard<nb::gil_scoped_release>());
}
