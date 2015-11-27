#ifndef METAAST_NAME_HPP
#define METAAST_NAME_HPP

#include <cassert>
#include <type_traits>

namespace metaast {
	template<typename T, int Nesting>
	struct EmptyBinder { using type = T; };

	///////////////////////////////////////////////////////////////////////////////

#if 0	//c++14
	template <char... chars>
	struct name : std::integer_sequence<char, chars...> {
		template<template<typename, int> class F = EmptyBinder, int Nesting = 0>
		using bind = typename F<name<chars...>, Nesting>::type;
		int operator()(void) { assert(false); return 0; }
	};

	template <typename T, T... chars>  
	constexpr name<chars...> operator""_n() { return {}; }

	#define NAME(x) decltype(#x##_n)
#else
template <char... chars>
struct name {
	template<template<typename, int> class F = EmptyBinder, int Nesting = 0>
	using bind = typename F<name<chars...>, Nesting>::type;
	int operator()(void) { assert(false); return 0; }
};
	//c++11 but VS does not support it
	//constexpr char to_char(const char* s) { return *s; }
	//#define NAME(x) name<to_char(#x)>
	#define NAME(x)	name<#@x>	//VS specific
#endif

	///////////////////////////////////////////////////////////////////////////////

	template <typename T>
	struct is_name : std::false_type {};
	template<char... chars>
	struct is_name<name<chars...>> : std::true_type {};
}

#endif