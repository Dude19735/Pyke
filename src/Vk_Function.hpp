#pragma once

#include <tuple>
#include <functional>

namespace VK4 {
    class Vk_Func {
    public:
        Vk_Func(int size) : _size(size) {}
        ~Vk_Func(){

        }

        virtual void operator()(std::function<void()> repeat){}

        std::shared_ptr<Vk_Func> get() {
            std::shared_ptr<void> x( new char[_size]);
            memcpy(x.get(), this, _size);

            return std::static_pointer_cast<Vk_Func>(x);
        }

    private:
        int _size;
    };

    template<class ObjType, class ... _Types>
    class Vk_TFunc : public Vk_Func {
        // function pointer type
        typedef void(ObjType::* mf_type)(std::function<void()>, _Types...);

        // recursion to unpack the arguments
        template <std::size_t... _Indices>
		struct _indices {};

		template <std::size_t _N, std::size_t... _Is>
		struct _build_indices : _build_indices<_N - 1, _N - 1, _Is...> {};

		template <std::size_t... _Is>
		struct _build_indices<0, _Is...> : _indices<_Is...> {};

    public:
        Vk_TFunc(ObjType* obj, mf_type func, std::tuple<_Types...>&& args)
        :
        _obj(obj),
        _func(func),
        _args(args),
        Vk_Func(sizeof(Vk_TFunc) + sizeof(Vk_Func))
        {}

        void operator()(std::function<void()> repeat) override {
            const _build_indices<std::tuple_size<std::tuple<_Types...>>::value> ind;
			_call_F(repeat, std::move(_args), ind);
        }

    private:
        mf_type _func;
		ObjType* _obj;
		std::tuple<_Types...> _args;

        template <typename _Args, std::size_t... _Inds>
		auto _call_F(std::function<void()> repeat, _Args&& args, const _indices<_Inds...>&) -> void
		{
			(_obj->*_func)(repeat, std::get<_Inds>(args)...);
		}
    };
}