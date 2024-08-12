#pragma once

#include <string>

namespace VK4 {
    class ObjectType {
	public:
		ObjectType(std::string type) : _type(type) {}
		const std::string& type() { return _type; }
	private:
		const std::string _type;
	};

	class ObjectType_P_C : public ObjectType { public: ObjectType_P_C(std::string type) :         ObjectType(type /* + "[p_c]"    */) {} };
	class ObjectType_PC : public ObjectType { public: ObjectType_PC(std::string type) :           ObjectType(type /* + "[pc]"     */) {} };
	class ObjectType_P_C_N : public ObjectType { public: ObjectType_P_C_N(std::string type) :     ObjectType(type /* + "[p_c_n]"  */) {} };
	class ObjectType_PCN : public ObjectType { public: ObjectType_PCN(std::string type) :         ObjectType(type /* + "[pcn]"    */) {} };
	class ObjectType_P_C_N_T : public ObjectType { public: ObjectType_P_C_N_T(std::string type) : ObjectType(type /* + "[p_c_n_t]"*/) {} };
	class ObjectType_PCNT : public ObjectType { public: ObjectType_PCNT(std::string type) :       ObjectType(type /* + "[pcnt]"   */) {} };
	class ObjectType_Info : public ObjectType { public: ObjectType_Info(std::string type) :       ObjectType(type /* + "[Info]"   */) {} };
}