#ifndef METAAST_NAMED_ELEMENTS_HPP
#define METAAST_NAMED_ELEMENTS_HPP

#include <metaast/variables.hpp>

#include <boost/mpl/if.hpp>

#include <tuple>
#include <type_traits>

namespace metaast {
	template<template<typename...> class Derived, typename... Ts>
	struct named_elements {
		using all = std::tuple<Ts...>;
		using types = std::tuple<typename Ts::type ...>;
		static const int size = sizeof...(Ts);

		template<template<typename, int> class F = EmptyBinder, int Nesting = 0>
		using bind = typename F<Derived<
			typename boost::mpl::if_<
				is_external_var<Ts>,
				Ts,
				typename Ts::template bind<F, Nesting>
			>::type ...
		>, Nesting>::type;

		template<typename Tname, int N> struct iterate_by_name {
			using curr_element = typename std::tuple_element<N - 1, all>::type;
			struct select_index { static const int value = N - 1; };
			using index = typename boost::mpl::if_<
				std::is_same<Tname, typename curr_element::name>,
				select_index,
				typename iterate_by_name<Tname, N - 1>::index
			>::type;
			using type = typename boost::mpl::if_<
				std::is_same<Tname, typename curr_element::name>,
				typename curr_element::type,
				typename iterate_by_name<Tname, N - 1>::type
			>::type;
		};
		template<typename Tname> struct iterate_by_name<Tname, 0> {
			struct index { static const int value = -1; };
			using type = void;
		};

		template<typename T, int N> struct iterate_by_type {
			using curr_element = typename std::tuple_element<N - 1, all>::type;
			struct select_index { static const int value = N - 1; };
			using index = typename boost::mpl::if_<
				std::is_same<T, curr_element>,
				select_index,
				typename iterate_by_type<T, N - 1>::index
			>::type;
			using type = typename boost::mpl::if_<
				std::is_same<T, curr_element>,
				typename curr_element::type,
				typename iterate_by_type<T, N - 1>::type
			>::type;
		};
		template<typename T> struct iterate_by_type<T, 0> {
			struct index { static const int value = -1; };
			using type = void;
		};

		template<typename Tname>
		struct ref_by_name {
			using formal = iterate_by_name<Tname, sizeof...(Ts)>;
			using type = typename formal::type;
			static const int index = formal::index::value;
		};

		template<typename T>
		struct ref_by_type {
			using formal = iterate_by_type<T, sizeof...(Ts)>;
			using type = typename formal::type;
			static const int index = formal::index::value;
		};
	};
}

#endif