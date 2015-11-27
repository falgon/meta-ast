#ifndef METAAST_STATEMENTS_HPP
#define METAAST_STATEMENTS_HPP

#include <boost/mpl/if.hpp>

#include <type_traits>

namespace metaast {
	template <typename Tcond, typename Tif, typename Telse = void>
	struct if_stmt {
		struct if_impl {
			template<template<typename, int> class F, int Nesting>
			using bind = typename F<if_stmt<
				typename Tcond::template bind<F, Nesting>,
				typename Tif::template bind<F, Nesting>
			>, Nesting>::type;

			template <typename... Tsts>
			void operator()(Tsts... sts) const { if (Tcond()(sts...)) Tif()(sts...); }
		};

		struct if_else_impl {
			template<template<typename, int> class F, int Nesting>
			using bind = typename F<if_stmt<
				typename Tcond::template bind<F, Nesting>,
				typename Tif::template bind<F, Nesting>,
				typename Telse::template bind<F, Nesting>
			>, Nesting>::type;

			template <typename... Tsts>
			void operator()(Tsts... sts) const {
				if (Tcond()(sts...))
					Tif()(sts...);
				else
					Telse()(sts...);
			}
		};
		
		using impl = typename boost::mpl::if_<
			std::is_void<Telse>,
			if_impl,
			if_else_impl
		>::type;

		template<template<typename, int> class F = EmptyBinder, int Nesting = 0>
		using bind = typename impl::template bind<F, Nesting>;

		template <typename... Tsts>
		void operator()(Tsts... sts) const { impl()(sts...); }
	};

	///////////////////////////////////////////////////////////////////////////////

	template <typename Tcond, typename Tstmt>
	struct while_stmt {
		template<template<typename, int> class F, int Nesting>
		using bind = typename F<while_stmt<
			typename Tcond::template bind<F, Nesting>,
			typename Tstmt::template bind<F, Nesting>
		>, Nesting>::type;

		template <typename... Tsts>
		void operator()(Tsts... sts) const {
			while (Tcond()(sts...)) {
				try { Tstmt()(sts...); }
				catch (break_stmt::exception&) { break; }
				catch (continue_stmt::exception&) { continue; }
			}
		}
	};

	///////////////////////////////////////////////////////////////////////////////

	template <typename Tinit, typename Tcond, typename Tinc, typename Tstmt>
	struct for_stmt {
		template<template<typename, int> class F, int Nesting>
		using bind = typename F<for_stmt<
			typename Tinit::template bind<F, Nesting>,
			typename Tcond::template bind<F, Nesting>,
			typename Tinc::template bind<F, Nesting>,
			typename Tstmt::template bind<F, Nesting>
		>, Nesting>::type;

		template <typename... Tsts>
		void operator()(Tsts... sts) const {
			for (Tinit()(sts...); Tcond()(sts...); Tinc()(sts...)) {
				try { Tstmt()(sts...); }
				catch (break_stmt::exception&) { break; }
				catch (continue_stmt::exception&) { continue; }
			}
		}
	};

	///////////////////////////////////////////////////////////////////////////////
}

#endif