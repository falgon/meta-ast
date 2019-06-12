#ifndef METAAST_CONSTS_HPP
#define METAAST_CONSTS_HPP

#include <metaast/name.hpp>

#include <type_traits>
#include <utility>

namespace metaast {
	template <typename T, T val>
	struct IntegralValue : std::integral_constant<T, val> {
		template<template<typename, int> class F = EmptyBinder, int Nesting = 0>
		using bind = typename F<IntegralValue<T, val>, Nesting>::type;

		T operator()(...) const { return val; }
	};

	#define INTEGRAL_VALUE_IMPL(_type, _name) template<_type val> using _name = IntegralValue<_type, val>;
	INTEGRAL_VALUE_IMPL(bool, Bool);
	INTEGRAL_VALUE_IMPL(char, Char);
	INTEGRAL_VALUE_IMPL(unsigned char, UnsignedChar);
	INTEGRAL_VALUE_IMPL(short, Short);
	INTEGRAL_VALUE_IMPL(unsigned short, UnsignedShort);
	INTEGRAL_VALUE_IMPL(int, Int);
	INTEGRAL_VALUE_IMPL(unsigned int, UnsignedInt);
	INTEGRAL_VALUE_IMPL(long, Long);
	INTEGRAL_VALUE_IMPL(unsigned long, UnsignedLong);
	#undef INTEGRAL_VALUE_IMPL

#if 0	//c++14
	namespace double_detail {
		template<typename T>
		static constexpr T pow(T x, int exp) {
			return  exp == 0 ? 1 :
					exp < 0  ? pow(1 / x, -exp) :
					x * pow(x, exp - 1);
		}

		template <char c, char... cs>
		struct count_integral_part {
			static constexpr int value = c == '.' ? 0 : 1 + count_integral_part<cs...>::value;
		};
		template<char c>
		struct count_integral_part<c> { static constexpr int value = c == '.' ? 0 : 1; };

		template <typename T, int rank, char c, char... cs>
		struct stod {
			static constexpr T value = (c == '.' ? 0 : (c - '0') * pow<T>(10, rank > 0 ? rank - 1 : rank)) + stod<T, rank - 1, cs...>::value;
		};

		template<typename T, int rank, char c>
		struct stod<T, rank, c> { static constexpr T value = c == '.' ? 0 : (c - '0') * pow<T>(10, rank > 0 ? rank - 1 : rank); };
	}

	template <typename T, char... chars>
	struct FloatingPointValue : public std::integer_sequence<char, chars...> {
		template<template<typename, int> class F = EmptyBinder, int Nesting = 0>
		using bind = typename F<FloatingPointValue<T, chars...>, Nesting>::type;

		static constexpr T value = double_detail::stod<T, double_detail::count_integral_part<chars...>::value, chars...>::value;
		T operator()(...) const { return value; }
	};

	template <char... chars>
	using Double = FloatingPointValue<double, chars...>;
	template <char... chars>
	using Float = FloatingPointValue<float, chars...>;

	template <char... chars>
	constexpr Double<chars...> operator""_d() { return {}; }
	#define DOUBLE(x) decltype(x##_d)

	template <char... chars>
	constexpr Float<chars...> operator""_f() { return {}; }
	#define FLOAT(x) decltype(x##_f)
#endif

#if 0	//c++14
	template <char... chars>
	struct String : std::integer_sequence<char, chars...> {
		template<template<typename, int> class F = EmptyBinder, int Nesting = 0>
		using bind = typename F<String<chars...>, Nesting>::type;
		static char value[sizeof...(chars) + 1];
		const char* operator()(...) const { return value; }
	};

	template <char... chars>
	char String<chars...>::value[sizeof...(chars) + 1] = {chars..., '\0'};

	template <char... chars>  
	constexpr String<chars...> operator""_s() { return {}; }

	#define STRING(x) decltype(x##_s)

#else
	template <char... chars>
	struct String {
		template<template<typename, int> class F = EmptyBinder, int Nesting = 0>
		using bind = typename F<String<chars...>, Nesting>::type;
		static char value[sizeof...(chars) + 1];
		const char* operator()(...) const { return value; }
	};

	template <char... chars>
	char String<chars...>::value[sizeof...(chars) + 1] = {chars..., '\0'};
#ifdef __MSC_VER
#   define STRING(x)	String<#@x>	//VS specific
#else
#   define STRING(x)    String<#x[0]>
#endif
#endif
}

#endif
