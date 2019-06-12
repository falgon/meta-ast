#ifndef METAAST_UTIL_HPP
#define METAAST_UTIL_HPP

#include <type_traits>

namespace metaast {
	//We must define these in C++11; in C++14 we can use the corresponding std versions (used a bit differently though)
	template<int ...> struct index_sequence {};
	template<int N, int ...S> struct make_index_sequence : make_index_sequence<N-1, N-1, S...> {};
	template<int ...S> struct make_index_sequence<0, S...>{ typedef index_sequence<S...> type; };

	///////////////////////////////////////////////////////////////////////////////

	template<int N, template<int> class F, typename Out = void>
	struct loop {
		template<typename... In>
		static void apply(Out* out, In... in) {
			loop<N - 1, F, Out>::apply(out, in...);
			F<N>::apply(out, in...);
		}
	};
	template<template<int N> class F, typename Out>
	struct loop<0, F, Out> { static void apply(...) {} };
}

#endif
