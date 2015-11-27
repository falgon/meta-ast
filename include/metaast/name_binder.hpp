#ifndef METAAST_NAME_BINDER_HPP
#define METAAST_NAME_BINDER_HPP

#include <metaast/name.hpp>

#include <boost/mpl/eval_if.hpp>
#include <boost/mpl/if.hpp>
#include <boost/mpl/and.hpp>

#include <type_traits>

namespace metaast {
	template<typename T, typename Vars, template<typename, int, int> class ScopeSpace, template<typename, int> class F, int Nesting>
	struct name_binder {
		template <typename T>
		struct has_var_by_name {
			static const int index = Vars::template ref_by_name<T>::index;
			using type = typename boost::mpl::if_c<index != -1, std::true_type, std::false_type>::type;
		};
		template <typename T>
		struct var_from_name {
			using var_type = typename Vars::template ref_by_name<T>::type;
			static const int index = Vars::template ref_by_name<T>::index;
			using type = ScopeSpace<var_type, Nesting, index>;
		};
		template <typename T>
		struct has_var_by_type {
			static const int index = Vars::template ref_by_type<T>::index;
			static const int value = index != -1;
			using type = typename boost::mpl::if_c<index != -1, std::true_type, std::false_type>::type;
		};
		template <typename T>
		struct var_from_type {
			using var_type = typename Vars::template ref_by_type<T>::type;
			static const int index = Vars::template ref_by_type<T>::index;
			using type = ScopeSpace<var_type, Nesting, index>;
		};
		using type = typename F<
			typename boost::mpl::eval_if<
				boost::mpl::and_<is_name<T>, has_var_by_name<T>>,
				var_from_name<T>,
				typename boost::mpl::if_<
					has_var_by_type<T>,
					var_from_type<T>,
					std::identity<T>
				>::type
			>::type,
			Nesting
		>::type;
	};
}

#endif