#ifndef METAAST_FORMALS_HPP
#define METAAST_FORMALS_HPP

#include <metaast/named_elements.hpp>

#include <tuple>

namespace metaast {
	template<typename Type, int index, bool method = true>
	struct formal {
		using type = Type;

		template<template<typename, int> class F = EmptyBinder, int Nesting = 0>
		using bind = typename F<formal<Type, index, method>, Nesting>::type;

		//within a method call, the first ST is self, then are the actuals
		template <typename Tself, typename Tactuals, typename... Tsts>
		auto operator() (Tself* self, Tactuals* actuals, Tsts... sts) const -> decltype(std::get<index>(*actuals))
			{ return std::get<index>(*actuals); }
	};
	template<typename Type, int index>
	struct formal<Type, index, false> {
		using type = Type;

		template<template<typename, int> class F = EmptyBinder, int Nesting = 0>
		using bind = typename F<formal<Type, index, false>, Nesting>::type;

		//within a function call, the first ST should always have the actuals
		template <typename Tactuals, typename... Tsts>
		auto operator() (Tactuals* actuals, Tsts... sts) const -> decltype(std::get<index>(*actuals))
			{ return std::get<index>(*actuals); }
	};

	///////////////////////////////////////////////////////////////////////////////

	template<typename... Ts>
	struct formals : named_elements<formals, Ts...> {};
}

#endif