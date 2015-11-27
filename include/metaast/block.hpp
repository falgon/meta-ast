#ifndef METAAST_BLOCK_HPP
#define METAAST_BLOCK_HPP

#include <metaast/name_binder.hpp>
#include <metaast/named_elements.hpp>
#include <metaast/util.hpp>

#include <tuple>
#include <type_traits>

namespace metaast {
	template<typename Type, int st_index, int var_index>
	struct local {
		using type = Type;

		template<template<typename, int> class F = EmptyBinder, int Nesting = 0>
		using bind = typename F<local<Type, st_index, var_index>, Nesting>::type;

		template <typename... Tsts>
		auto operator() (Tsts... sts) const -> decltype(std::get<var_index>(*std::get<st_index>(std::make_tuple(sts...)))) {
			return std::get<var_index>(*std::get<st_index>(std::make_tuple(sts...))); }
	};

	///////////////////////////////////////////////////////////////////////////////

	template<typename... Ts>
	struct locals : named_elements<locals, Ts...> {};

	///////////////////////////////////////////////////////////////////////////////

	template <typename TLocals, typename... Tstmts>
	struct block {
		using locals_environment = typename TLocals::types;

		template<template<typename, int> class F, int Nesting>
		struct bind_helper {
			template<typename T, int N>
			using BoundNameBinder = name_binder<T, TLocals, local, F, Nesting>;
			//template<typename T, int N>
			//using BoundNameBinder = name_binder<T, typename TLocals::template bind<BoundNameBinder1, Nesting>, local, F, Nesting>;
			using type = typename F<
				block<
					typename TLocals::template bind<BoundNameBinder, Nesting>,
					typename Tstmts::template bind<BoundNameBinder, Nesting> ...
				>,
				Nesting
			>::type;
		};
		
		template<template<typename, int> class F = EmptyBinder, int Nesting = 0>
		using bind = typename bind_helper<F, Nesting + 1>::type;

		template<int N>
		struct locals_loop_iteration {
			using Tlocal = typename std::tuple_element<N - 1, typename TLocals::all>::type;
			template<typename... In>
			static void apply(locals_environment* out, In... in) { std::get<N - 1>(*out) = Tlocal()(in...); }
		};

		template<int N>
		struct stmts_loop_iteration {
			using type = typename std::tuple_element<N - 1, std::tuple<Tstmts...>>::type;
			template <typename... In>
			static void apply(void*, In... in) { type()(in...); }
		};

		template <typename... Tsts>
		void operator()(Tsts... sts) const {
			locals_environment st;
			loop<std::tuple_size<locals_environment>::value, locals_loop_iteration, locals_environment>::apply(&st, sts...);
			loop<sizeof...(Tstmts), stmts_loop_iteration>::apply(nullptr, sts..., &st);
		}
		//void operator()(void) const {
		//	locals st;
		//	loop<std::tuple_size<locals>::value, locals_loop_iteration, locals>::apply(&st);
		//	loop<sizeof...(Tstmts), stmts_loop_iteration>::apply(&st);
		//}
	};
}

#endif