#include <nanobind/nanobind.h>
#include <nanobind/stl/bind_vector.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/tuple.h>
#include <nanobind/stl/set.h>
#include <nanobind/stl/map.h>
#include <nanobind/ndarray.h>
#include <nanobind/stl/shared_ptr.h>

#include <vector>
#include <memory>
#include <iostream>
#include <span>
#include <exception>
#include <format>

// #ifdef PYVK
// namespace py = pybind11;
namespace nb = nanobind;

struct Struct {
	int C = 0;
	int X = 0;
	int Y = 0;
	int Z = 0;
};

struct Struct2 {
	int C = 0;
	int X = 0;
	int Y = 0;
	int Z = 0;
};

class Test {
	static inline int counter = 0;
public:
	int C;
	int X;
	int Y;
	int Z;
	int* array;
	Test(int x, int y, int z) : C(counter++), X(x), Y(y), Z(z), array(new int[10]) {
		std::cout << "Create constructor: " << C << std::endl;
	}
	Test(const Test& other) 
	: 
	C(counter++),
	X(other.X),
	Y(other.Y),
	Z(other.Z),
	array(new int[10])
	{
		std::cout << "Copy constructor: " << C << " (other: " << other.C << ")" << std::endl;
		memcpy(array, other.array, 10*sizeof(int));

	}
	Test(Test&& other) 
	: 
	C(counter++),
	X(other.X),
	Y(other.Y),
	Z(other.Z),
	array(other.array) 
	{
		std::cout << "Move constructor: " << C << " (other: " << other.C << ")" << std::endl;
		other.array = nullptr;
	}
	~Test(){
		std::cout << "Destructor: " << C << std::endl;
		if(array != nullptr) {
			std::cout << " ... delete array: " << C << std::endl;
			delete[] array;
		}
	}
	Test operator=(const Test& other) noexcept {
		C = counter++;
		std::cout << "Copy assignment operator: " << C << " = " << other.C << std::endl;
		if(this == &other) return std::move(*this);
		X = other.X;
		Y = other.Y;
		Z = other.Z;
		array = new int[10];
		memcpy(array, other.array, 10*sizeof(int));
		return std::move(*this);
	}
	Test operator=(Test&& other) noexcept {
		C = counter++;
		std::cout << "Move assignment operator: " << C << " <- " << other.C << std::endl;
		if(this == &other) return std::move(*this);
		X = other.X;
		Y = other.Y;
		Z = other.Z;
		array = other.array;
		other.array = nullptr;
		return std::move(*this);
	}

	std::string to_string() const {
		int bla = static_cast<int>(array!=nullptr);
		return std::vformat("[{0},{1},{2}], counter: {3}, array exists: {4}", std::make_format_args(X,Y,Z,C, bla));
	}

	int get(int index){
		if(index > 9 || index < 0) throw new std::out_of_range("index");
		return array[index];
	}
};

NB_MODULE(_pyke, m) {
	m.doc() = "Test module for Nanobind"; // optional module docstring

	// nb::class_<Struct2>(m, "Struct2")
	// 	.def("__init__", [](std::shared_ptr<Struct2> self, int x, int y, int z){
	// 		// this one will output random stuff for self.C => self doesn't exist at this point
	// 		// => this one can safely be used for initialization of Structs that don't have a constructor.
	// 		self = std::make_shared<Struct2>(Struct2{.C=1, .X=x, .Y=y, .Z=z});
	// 	})
	// 	.def_rw("C", &Struct::C)
	// 	.def_rw("X", &Struct::X)
	// 	.def_rw("Y", &Struct::Y)
	// 	.def_rw("Z", &Struct::Z);

	nb::class_<Struct>(m, "Struct")
		.def("__init__", [](Struct& self, int x, int y, int z){
			// this one will output random stuff for self.C => self doesn't exist at this point
			// => this one can safely be used for initialization of Structs that don't have a constructor.
			self = Struct{.C=self.C+1, .X=x, .Y=y, .Z=z};
		})
		.def_rw("C", &Struct::C)
		.def_rw("X", &Struct::X)
		.def_rw("Y", &Struct::Y)
		.def_rw("Z", &Struct::Z);

	m.def("make_test", [](int x, int y, int z){
		// This one calls the move constructor twice => make sure that stuff works for this kind of semantics!!
		return std::move(Test(x,y,z));
	});

	nb::class_<Test>(m, "Test")
		/* =================================================================================== */
		/* This one is the regular way of doing things */
		// .def(nb::init<int, int, int>(),
		// 	nb::arg("x"), nb::arg("y"), nb::arg("z"),
		// 	nb::rv_policy::reference_internal,
		// 	nb::call_guard<nb::gil_scoped_release>())
		/* =================================================================================== */
		/* =================================================================================== */
		/* This one has a wild sequence of calling constructors and assignments... Avoid it!!! */
		// .def("__init__", [](Test& self, int x, int y, int z) -> void {
		// 		std::cout << "lol1" << std::endl;
		// 		self = std::move(Test(x, y, z));
		// 		std::cout << "lol2" << std::endl;
		// 		std::cout << "lol3" << std::endl;
		// 		// return std::move(self);
		// 	}, nb::arg("x"), nb::arg("y"), nb::arg("z"),
		// 	nb::rv_policy::reference_internal,
		// 	nb::call_guard<nb::gil_scoped_release>())
		/* =================================================================================== */
		/* =================================================================================== */
		/* This one is a work of art!! */
		.def(nb::new_([](int x, int y, int z) {
				std::cout << "lol1" << std::endl;
				return std::make_shared<Test>(Test(x, y, z));
				// std::cout << "lol2" << std::endl;
				// return std::move(self);
			}), nb::arg("x"), nb::arg("y"), nb::arg("z"),
			nb::call_guard<nb::gil_scoped_release>())
		/* =================================================================================== */
		.def("test_func", [](
			Test& self,
			int new_val
		){
			self.X = new_val;
		}, nb::arg("new_val"))
		.def("get", &Test::get, nb::arg("index"), nb::rv_policy::copy)
		.def_rw("X", &Test::X, nb::rv_policy::copy)
		.def_rw("Y", &Test::Y, nb::rv_policy::copy)
		.def_rw("Z", &Test::Z, nb::rv_policy::copy)
		.def("__str__", [](const Test& self){return self.to_string();});

	m.def("inspect", [](const nb::ndarray<>& a) {
		printf("Array data pointer : %p\n", a.data());
		printf("Array dimension : %zu\n", a.ndim());
		for (size_t i = 0; i < a.ndim(); ++i) {
			printf("Array dimension [%zu] : %zu\n", i, a.shape(i));
			printf("Array stride    [%zu] : %zd\n", i, a.stride(i));
		}
		printf("Device ID = %u (cpu=%i, cuda=%i)\n", a.device_id(),
			int(a.device_type() == nb::device::cpu::value),
			int(a.device_type() == nb::device::cuda::value)
		);
		printf("Array dtype: int16=%i, uint32=%i, float32=%i\n",
			a.dtype() == nb::dtype<int16_t>(),
			a.dtype() == nb::dtype<uint32_t>(),
			a.dtype() == nb::dtype<float>()
		);
	});

	m.def("cpp_print", 
		[](const nb::ndarray<const float, nb::ndim<1>, nb::c_contig, nb::device::cpu>& a) {
			auto x = a.data();
			size_t s = a.size();
			std::vector<int> vec(s);
			memcpy(vec.data(), x, s*sizeof(int));
			std::cout << "size: " << s << std::endl;
			for(size_t i=0; i<s; ++i){
				std::cout << i << ": " << vec[i] << std::endl;
			}
		},
		nb::arg("a")
	);

	m.def("cpp_vec", [](const nb::ndarray<const float, nb::ndim<1>, nb::c_contig, nb::device::cpu>& a) {
		std::cout << "cpp_vec" << std::endl;
		std::span<const float> vec(reinterpret_cast<const float*>(a.data()), a.size());
		std::cout << "size: " << vec.size() << std::endl;
		for(size_t i=0; i<vec.size(); ++i){
			std::cout << i << ": " << vec[i] << std::endl;
		}
	},nb::arg("a"));
}