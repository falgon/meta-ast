#ifndef METAAST_VARIABLES_HPP
#define METAAST_VARIABLES_HPP

#include <metaast/name.hpp>

#include <boost/mpl/eval_if.hpp>

#include <type_traits>

namespace metaast {
	template <template<typename, typename, typename> class Derived, typename Type, typename Name, typename Value>
	struct var_base {
		using type = Type;
		using name = Name;

		template<template<typename, int> class F, int Nesting>
		struct bind_named_type { using type = typename Type::template bind<F, Nesting>; };
		
		template<template<typename, int> class F = EmptyBinder, int Nesting = 0>
		using bind_type = typename boost::mpl::eval_if<
			is_name<Type>,
			bind_named_type<F, Nesting>,
			std::identity<Type>
		>::type;

		template<template<typename, int> class F, int Nesting>
		struct bind_non_void_value { using type = typename Value::template bind<F, Nesting>; };

		struct void_wrapper { using type = void; };
		template<template<typename, int> class F = EmptyBinder, int Nesting = 0>
		using bind_value = typename boost::mpl::eval_if<
			std::is_void<Value>,
			void_wrapper,
			bind_non_void_value<F, Nesting>
		>::type;

		//do not bind the name here
		template<template<typename, int> class F = EmptyBinder, int Nesting = 0>
		using bind = typename Derived<bind_type<F, Nesting>, Name, bind_value<F, Nesting>>;
	};

	template <typename Type, typename Name, typename Value = void>
	struct var : var_base<var, Type, Name, Value> {
		template <typename... Tsts>
		Type operator()(Tsts... sts) const { return Value()(sts...); }
	};
	template <typename Type, typename Name>
	struct var<Type, Name, void> : var_base<var, Type, Name, void> {
		template <typename... Tsts>
		Type operator()(Tsts... sts) const { return type(); }
	};

	///////////////////////////////////////////////////////////////////////////////
	
	template <typename T>
	struct external_var {
		using type = T;
		using name = void;

		template<template<typename, int> class F = EmptyBinder, int Nesting = 0>
		using bind = typename F<external_var<T>, Nesting>::type;

		T operator()(...) { return T(); }
	};

	template <typename T>
	struct is_external_var : std::false_type {};
	template<typename T>
	struct is_external_var<external_var<T>> : std::true_type {};
}

#endif