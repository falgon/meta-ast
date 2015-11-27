#ifndef METAAST_CALLS_HPP
#define METAAST_CALLS_HPP

#include <metaast/functions.hpp>
#include <metaast/name.hpp>
#include <metaast/util.hpp>

#include <boost/mpl/eval_if.hpp>
#include <boost/mpl/if.hpp>
#include <boost/mpl/or.hpp>

#include <tuple>
#include <type_traits>

namespace metaast {
	template <typename Tfunc, typename... Targs>
	struct call_expr {
		using Types			= std::tuple<Targs...>;
		using result_type	= typename function_traits<Tfunc>::result_type;
		using arguments		= typename function_traits<Tfunc>::arguments;

		template<template<typename, int> class F, int Nesting>
		struct bind_func_def_or_method {
			using type = typename Tfunc::template bind<F, Nesting>;
		};

		template<template<typename, int> class F, int Nesting>
		using bind_func = typename boost::mpl::eval_if<
			boost::mpl::or_<is_func_def<Tfunc>, is_method<Tfunc>, is_name<Tfunc>>,
			bind_func_def_or_method<F, Nesting>,
			std::identity<Tfunc>
		>::type;

		template<template<typename, int> class F = EmptyBinder, int Nesting = 0>
		using bind = typename F<call_expr<bind_func<F, Nesting>, typename Targs::template bind<F, Nesting> ...>, Nesting>::type;

		template<int N>
		struct param_loop_iteration {
			using Tactual = typename std::tuple_element<N - 1, Types>::type;
			template<typename... In>
			static void apply(arguments* out, In... in) { std::get<N - 1>(*out) = Tactual()(in...); }
		};

		struct func_def_impl {
			template <typename Args, typename... Tsts>
			result_type operator()(Args* actuals, Tsts...) const { return Tfunc().call(actuals); }
		};

		struct functor_and_funcptr_impl {
			template <typename Args, int... I>
			result_type operator()(Args* actuals, index_sequence<I...>) const { return Tfunc()(std::get<I>(*actuals)...); }
			template <typename Args, typename... Tsts>
			result_type operator()(Args* actuals, Tsts...) const { return operator()(actuals, typename make_index_sequence<sizeof...(Targs)>::type()); }
		};
		
		using impl = typename boost::mpl::if_<
			is_method<Tfunc>,
			Tfunc,
			typename boost::mpl::if_<
				is_func_def<Tfunc>,
				func_def_impl,
				functor_and_funcptr_impl
			>::type
		>::type;

		template <typename... Tsts>
		result_type operator()(Tsts... sts) const { 
			arguments actuals;
			loop<sizeof ...(Targs), param_loop_iteration, arguments>::apply(&actuals, sts...);
			return impl()(&actuals, sts...);
		}
	};

}

#endif