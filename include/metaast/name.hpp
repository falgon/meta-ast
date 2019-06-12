#ifndef METAAST_NAME_HPP
#define METAAST_NAME_HPP

#include <cassert>
#include <type_traits>
#if __cplusplus >= 201402L
#   include <utility>
#endif

namespace metaast {
	template<typename T, int Nesting>
	struct EmptyBinder { using type = T; };

	///////////////////////////////////////////////////////////////////////////////

#if __cplusplus >= 201402L
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
#ifdef __MSC_VER
#   define NAME(x) name<#@x>
#elif __cplusplus >= 201103L
namespace {
    constexpr char to_char(const char* s) { return *s; }
} // anonymouse namespace
	#define NAME(x)	name<to_char(#x)>	
#else
#   error This feature needs to be C++11
#endif
#endif

	///////////////////////////////////////////////////////////////////////////////

	template <typename T>
	struct is_name : std::false_type {};
	template<char... chars>
	struct is_name<name<chars...>> : std::true_type {};
}

#endif
