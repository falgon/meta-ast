#ifndef METAAST_JUMP_STATEMENTS_HPP
#define METAAST_JUMP_STATEMENTS_HPP

#include <cassert>
#include <type_traits>

namespace metaast {
	struct return_exception_with_value {};
	template <typename T>
	struct return_exception : public return_exception_with_value {
		T retval;
		return_exception(T val) : retval(val) {}
	};
	template <>
	struct return_exception<void> {};

	template <typename T>
	return_exception<T> make_return_exception(T val) { return return_exception<T>(val); }
	return_exception<void> make_return_exception(void) { return return_exception<void>(); }

	///////////////////////////////////////////////////////////////////////////////

	template <typename Tret = void>
	struct return_stmt {
		template<template<typename, int> class F = EmptyBinder, int Nesting = 0>
		using bind = typename F<return_stmt<typename Tret::template bind<F, Nesting>>, Nesting>::type;

		template <typename... Tsts>
		void operator()(Tsts... sts) const { throw make_return_exception(Tret()(sts...)); }
	};
	template <>
	struct return_stmt<void> {
		template<template<typename, int> class F = EmptyBinder, int Nesting = 0>
		using bind = typename F<return_stmt<void>, Nesting>::type;

		template <typename... Tsts>
		void operator()(Tsts... sts) const { throw return_exception<void>(); }
	};

	template <typename T>
	struct is_return_stmt : std::false_type {};
	template<typename Tret>
	struct is_return_stmt<return_stmt<Tret>> : std::true_type {};

	///////////////////////////////////////////////////////////////////////////////

	struct break_stmt {
		struct exception {};

		template<template<typename, int> class F = EmptyBinder, int Nesting = 0>
		using bind = typename F<break_stmt, Nesting>::type;

		template <typename... Tsts>
		void operator()(Tsts... sts) const { throw exception(); }
	};

	///////////////////////////////////////////////////////////////////////////////

	struct continue_stmt {
		struct exception {};

		template<template<typename, int> class F = EmptyBinder, int Nesting = 0>
		using bind = typename F<continue_stmt, Nesting>::type;

		template <typename... Tsts>
		void operator()(Tsts... sts) const { throw exception(); }
	};
}

#endif