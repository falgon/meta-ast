#ifndef METAAST_PROGRAM_HPP
#define METAAST_PROGRAM_HPP

#include <metaast/name.hpp>
#include <metaast/name_binder.hpp>
#include <metaast/named_elements.hpp>

#include <boost/mpl/eval_if.hpp>
#include <boost/mpl/if.hpp>

#include <type_traits>

namespace metaast {
	template<typename... Ts>
	struct defs : named_elements<defs, Ts...> {};

	///////////////////////////////////////////////////////////////////////////////

	template<typename Tdefs, typename Tcode>
	struct program {
		template<typename TT, typename Vars, template<typename, int> class F, int Nesting>
		struct custom_name_binder {
			template <typename T>
			struct has_var_by_name {
				static const int index = Vars::template ref_by_name<T>::index;
				using type = typename boost::mpl::if_c<index != -1, std::true_type, std::false_type>::type;
			};
			template <typename T>
			struct var_from_name { using type = typename Vars::template ref_by_name<T>::type::template bind<>; };
			template <typename T>
			struct has_var_by_type {
				static const int index = Vars::template ref_by_type<T>::index;
				static const int value = index != -1;
				using type = typename boost::mpl::if_c<index != -1, std::true_type, std::false_type>::type;
			};
			template <typename T>
			struct var_from_type { using type = typename Vars::template ref_by_type<T>::type; };
			using type = typename F<
				typename boost::mpl::eval_if<
					boost::mpl::and_<is_name<TT>, has_var_by_name<TT>>,
					var_from_name<TT>,
					typename boost::mpl::if_<
						has_var_by_type<TT>,
						var_from_type<TT>,
						boost::mpl::identity<TT>
					>::type
				>::type,
				Nesting
			>::type;
		};
			
        template<int N, template <typename, int> class F, int Nesting>
        struct bind_loop {
            template<typename T, int>
            using type = custom_name_binder<T, typename Tdefs::template bind<bind_loop<N - 1, F, Nesting>::template type, Nesting>, F, Nesting>;
        };
	    template<template <typename, int> class F, int Nesting>
		struct bind_loop<0, F, Nesting> {
			template<typename T, int>
			using type = custom_name_binder<T, Tdefs, F, Nesting>;
		};

		template<template<typename, int> class F, int Nesting>
		struct bind_helper {
			template<typename T, int N>
			using BoundNameBinder = typename bind_loop<Tdefs::size, F, Nesting>::template type<T, N>;
			using type = typename F<program<typename Tdefs::template bind<BoundNameBinder, Nesting>, typename Tcode::template bind<BoundNameBinder, Nesting>>, Nesting>::type;
		};

		template<template<typename, int> class F = EmptyBinder>
		using bind = typename bind_helper<F, 0>::type;

		void operator()() { Tcode()(); }
	};
}

#endif
