#ifndef METAAST_FUNCTIONS_HPP
#define METAAST_FUNCTIONS_HPP

#include <metaast/formals.hpp>
#include <metaast/jump_statements.hpp>
#include <metaast/name_binder.hpp>

#include <tuple>
#include <type_traits>

namespace metaast {
	template <typename T>
	struct function_traits;

	///////////////////////////////////////////////////////////////////////////////

	template <template<typename, typename, typename, typename> class Derived, typename Tret, typename Tname, typename TFormals, typename Tbody, bool isMethod>
	struct func_def_base {
		using type = Derived<Tret, Tname, TFormals, Tbody>;
		using name = Tname;

		using result_type = Tret;
		using arguments = typename TFormals::types;
		using formals_type = TFormals;
		using body_type = Tbody;

		template<template<typename, int> class F, int Nest>
		struct bind_helper {
			template<typename T, int Nesting, int index>
			using FormalWrapper = formal<T, index, isMethod>;
			template<typename T, int N>
			using BoundNameBinder = name_binder<T, TFormals, FormalWrapper, F, Nest>;
			//Binding applies only to function body
			using type = typename F<Derived<Tret, Tname, TFormals, typename Tbody::template bind<BoundNameBinder, Nest>>, Nest>::type;
		};
		
		template<template<typename, int> class F = EmptyBinder, int Nesting = 0>
		using bind = typename bind_helper<F, Nesting>::type;
	};

	template <typename Tret, typename Tname, typename TFormals, typename Tbody>
	struct func_def : public func_def_base<func_def, Tret, Tname, TFormals, Tbody, false> {
		template <typename T>
		Tret call(T* actuals) const {
			try { Tbody()(actuals); }
			catch (return_exception<void>&) { /*TODO: return without a value*/ }
			catch (return_exception<Tret>& e) { return e.retval; }
			catch (...) { throw; }
			//runtime warning: function does not return value
			return Tret();
		}
		template<typename... Args>
		Tret operator()(Args... args) {
			auto t = std::make_tuple(args...);
			return call(&t);
		}
	};
	template <typename Tname, typename TFormals, typename Tbody>
	struct func_def<void, Tname, TFormals, Tbody> : public func_def_base<func_def, void, Tname, TFormals, Tbody, false> {
		template <typename T>
		void call(T* actuals) const {
			try { Tbody()(actuals); } 
			catch (return_exception<void>&) {}
			catch (return_exception_with_value&) { /*TODO: void function returns a value*/}
			catch (...) { throw; }
		}
		template<typename... Args>
		void operator()(Args... args) {
			auto t = std::make_tuple(args...);
			call(&t);
		}
	};

	///////////////////////////////////////////////////////////////////////////////

	template<typename TFunc, TFunc F>
	struct extern_func {
		template<typename... Args>
		typename function_traits<TFunc>::result_type operator()(Args... args) { return F(args...); }
	};
	
	#define EXTERN_FUNC(f)							extern_func<std::add_pointer_t<decltype(f)>, f>
	#define EXTERN_OVERLOADED_FUNC(signature, f)	extern_func<decltype(static_cast<signature>(f)), &f>

	///////////////////////////////////////////////////////////////////////////////

	template <typename T>
	struct is_method : std::false_type {};

	template <typename T>
	struct is_func_def : std::false_type {};
	template<typename Tret, typename Tname, typename TFormals, typename Tstmts>
	struct is_func_def<func_def<Tret, Tname, TFormals, Tstmts>> : std::true_type {};

	///////////////////////////////////////////////////////////////////////////////
	// function_traits
	//
	template <typename T>
	struct function_traits : public function_traits<decltype(&T::operator())> {};

	template <typename Tret, typename C, typename... Args>
	struct function_traits<Tret (C::*)(Args...)> : public function_traits<Tret (C::*)(Args...) const> {};

	template <typename Tret, typename C, typename... Args>
	struct function_traits<Tret (C::*)(Args...) const> {
		using result_type = Tret;
		using arguments = std::tuple<typename std::decay<Args>::type...>;	//TODO: recheck this
	};
 
	template<typename Tret, typename... Args>
	struct function_traits<Tret(*)(Args...)> : public function_traits<Tret(Args...)> {};
 
	template<typename Tret, typename... Args>
	struct function_traits<Tret(Args...)> {
		using result_type = Tret;
		using arguments = std::tuple<typename std::decay<Args>::type...>;	//TODO: recheck this
	};

	template <typename Tret, typename Tname, typename TFormals, typename Tbody>
	struct function_traits<func_def<Tret, Tname, TFormals, Tbody>> {
		using result_type = Tret;
		using arguments = typename func_def<Tret, Tname, TFormals, Tbody>::arguments;
	};

	template <typename TFunc, TFunc F>
	struct function_traits<extern_func<TFunc, F>> {
		using result_type	= typename function_traits<TFunc>::result_type;
		using arguments		= typename function_traits<TFunc>::arguments;
	};

}

#endif
