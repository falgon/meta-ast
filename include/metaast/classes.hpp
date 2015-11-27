#ifndef METAAST_CLASSES_HPP
#define METAAST_CLASSES_HPP

#include <metaast/functions.hpp>
#include <metaast/name.hpp>
#include <metaast/name_binder.hpp>
#include <metaast/util.hpp>

#include <boost/mpl/eval_if.hpp>
#include <boost/mpl/if.hpp>

namespace metaast {
	#define CLASSOF(_obj) _obj::type

	///////////////////////////////////////////////////////////////////////////////

	template <typename Type, int Index>
	struct member_index {
		using type = Type;
		static const int index = Index;

		template<template<typename, int> class F = EmptyBinder, int Nesting = 0>
		using bind = typename F<member_index<Type, Index>, Nesting>::type;

		template <typename Tself, typename... Tsts>
		Type operator()(Tself* self, Tsts... sts) const { return std::get<Index>(*self); }
	};

	///////////////////////////////////////////////////////////////////////////////

	template <typename Tobj, typename Tmem>
	struct member {
		template<typename Tclass, typename T>
		struct MemberNameBinder {
			template<typename T>
			struct get_members { using type = typename T::members; };
			struct get_empty_members {
				struct type {
					template<typename T> struct ref_by_name {
						static const int index = -1;
						using type = void;
					};
				};
			};

			using TMembers = typename boost::mpl::eval_if<
				is_name<Tclass>,
				get_empty_members,
				get_members<Tclass>
			>::type;

			template <typename T>
			struct has_member {
				static const int index = TMembers::template ref_by_name<T>::index;
				using type = typename boost::mpl::if_c<index != -1, std::true_type, std::false_type>::type;
			};
			template <typename T>
			struct member_from_name {
				using member_type = typename TMembers::template ref_by_name<T>::type;
				static const int index = TMembers::template ref_by_name<T>::index;
				using type = member_index<member_type, index>;
			};
			using type = typename boost::mpl::eval_if<
				typename has_member<T>::type,
				member_from_name<T>,
				std::identity<T>
			>::type;
		};
		template<template<typename, int> class F = EmptyBinder, int Nesting = 0>
		using bind = typename F<member<typename Tobj::template bind<F, Nesting>, typename MemberNameBinder<typename Tobj::template bind<F, Nesting>::type, Tmem>::type>, Nesting>::type;

		template <typename... Tsts>
		auto operator()(Tsts... sts) const ->decltype(std::get<Tmem::index>(Tobj()(sts...)))
			{ return std::get<Tmem::index>(Tobj()(sts...)); }
	};

	#define MEMBER(o, mem)	member<NAME(o), NAME(mem)>

	///////////////////////////////////////////////////////////////////////////////

	template <typename Tobj, typename Tclass, typename Ttype, Ttype (Tclass::*mem)>
	struct extern_member {
		template<template<typename, int> class F = EmptyBinder, int Nesting = 0>
		using bind = typename F<extern_member<typename Tobj::template bind<F, Nesting>, Tclass, Ttype, mem>, Nesting>::type;

		template <typename... Tsts>
		Ttype& operator()(Tsts... sts) const { return Tobj()(sts...).*mem; }
	};

	template <class T, class M> M _member_type(M T:: *);

	#define EXTERN_MEMBER(_obj, _mem) extern_member<_obj, CLASSOF(_obj), decltype(_member_type(&CLASSOF(_obj)::_mem)), &CLASSOF(_obj)::_mem>

	///////////////////////////////////////////////////////////////////////////////

	template <typename Tclass, typename Tmethod>
	struct method {
		template<template<typename, int> class F = EmptyBinder, int Nesting = 0>
		using bind = typename F<method<typename Tclass::template bind<F, Nesting>, typename Tmethod::template bind<F, Nesting>>, Nesting>::type;

		template <typename Args, typename... Tsts>
		auto operator()(Args* actuals, Tsts... sts) const -> decltype(Tmethod().call(&Tclass()(sts...), actuals))
			{ return Tmethod().call(&Tclass()(sts...), actuals); }
	};

	///////////////////////////////////////////////////////////////////////////////

	template <typename ObjName, typename MethodName>
	struct unresolved_method {
		template<typename Tclass, typename T>
		struct MethodNameBinder {
			template<typename T>
			struct get_methods { using type = typename T::methods; };
			struct get_empty_methods {
				struct type {
					template<typename T> struct ref_by_name {
						static const int index = -1;
						using type = void;
					};
				};
			};

			using TMethods = typename boost::mpl::eval_if<
				is_name<Tclass>,
				get_empty_methods,
				get_methods<Tclass>
			>::type;

			template <typename T>
			struct has_method {
				static const int index = TMethods::template ref_by_name<T>::index;
				using type = typename boost::mpl::if_c<index != -1, std::true_type, std::false_type>::type;
			};
			template <typename T>
			struct method_from_name { using type = typename TMethods::template ref_by_name<T>::type::template bind<>; };
			using type = typename boost::mpl::eval_if<
				typename has_method<T>::type,
				method_from_name<T>,
				std::identity<T>
			>::type;
		};
		template<template<typename, int> class F = EmptyBinder, int Nesting = 0>
		using bind = typename F<method<typename ObjName::template bind<F, Nesting>, typename MethodNameBinder<typename ObjName::template bind<F, Nesting>::type, MethodName>::type>, Nesting>::type;
		//int operator()(...) { assert(false); return 0; }
	};

	#define METHOD(o, method)	unresolved_method<NAME(o), NAME(method)>

	///////////////////////////////////////////////////////////////////////////////

	template <typename Tobj, typename Tmethod>
	struct is_method<method<Tobj, Tmethod>> : std::true_type {};
	template <typename Tobj, typename Tmethod>
	struct is_method<unresolved_method<Tobj, Tmethod>> : std::true_type {};

	template <typename Tobj, typename Tmethod>
	struct function_traits<method<Tobj, Tmethod>> : function_traits<Tmethod> {};

	template <typename Tobj, typename Tmethod>
	struct function_traits<unresolved_method<Tobj, Tmethod>> {
		struct unresolved_type {};
		using result_type	= unresolved_type;
		using arguments		= std::tuple<unresolved_type>;
	};

	///////////////////////////////////////////////////////////////////////////////

	template <typename Tobj, typename Tclass, typename T, T func>
	struct extern_method {
		template<template<typename, int> class F = EmptyBinder, int Nesting = 0>
		using bind = typename F<extern_method<typename Tobj::template bind<F, Nesting>, Tclass, T, func>, Nesting>::type;

		template <typename Args, typename... Tsts>
		typename function_traits<T>::result_type operator()(Args* actuals, Tsts... sts) const
			{ return operator()(Tobj()(sts...), actuals, typename make_index_sequence<std::tuple_size<Args>::value>::type()); }

	private:
		template <typename Args, int... I>
		typename function_traits<T>::result_type operator()(Tclass& obj, Args* actuals, index_sequence<I...>) const
			{ return (obj.*func)(std::get<I>(*actuals)...); }
	};

	template <typename Tobj, typename Tclass, typename TFunc, TFunc F>
	struct is_method<extern_method<Tobj, Tclass, TFunc, F>> : std::true_type {};

	template <typename Tobj, typename Tclass, typename TFunc, TFunc F>
	struct function_traits<extern_method<Tobj, Tclass, TFunc, F>> {
		using result_type	= typename function_traits<TFunc>::result_type;
		using arguments		= typename function_traits<TFunc>::arguments;
	};

	#define EXTERN_METHOD(_obj, _f)					extern_method<_obj, CLASSOF(_obj), decltype(&CLASSOF(_obj)::_f), &CLASSOF(_obj)::_f>
	#define OVERLOADED_METHOD(_obj, _signature, _f)	extern_method<_obj, CLASSOF(_obj), decltype(_signature(&CLASSOF(_obj)::_f)), &CLASSOF(_obj)::_f>

	///////////////////////////////////////////////////////////////////////////////

	template <typename Tret, typename Tname, typename TFormals, typename Tbody>
	struct method_def : public func_def_base<method_def, Tret, Tname, TFormals, Tbody, true> {
		template <typename Tself, typename Tactuals>
		Tret call(Tself* self, Tactuals* actuals) const { 
			try { Tbody()(self, actuals); }
			catch (return_exception<void>&) { /*TODO: return without a value*/ }
			catch (return_exception<Tret>& e) { return e.retval; }
			catch (...) { throw; }
			//runtime warning: method does not return value
			return Tret();
		}
		template<typename Tself, typename... Args>
		Tret operator()(Tself* self, Args... args) { return call(self, std::make_tuple(args...)); }
	};

	template <typename Tname, typename TFormals, typename Tbody>
	struct method_def<void, Tname, TFormals, Tbody> : public func_def_base<method_def, void, Tname, TFormals, Tbody, true> {
		template <typename Tself, typename Tactuals>
		void call(Tself* self, Tactuals* actuals) const { 
			try { Tbody()(self, actuals); } 
			catch (return_exception<void>&) {}
			catch (return_exception_with_value&) { /*TODO: void method returns a value*/ }
			catch (...) { throw; }
		}
		template<typename Tself, typename... Args>
		void operator()(Tself* self, Args... args) { call(self, std::make_tuple(args...)); }
	};

	template <typename Tret, typename Tname, typename TFormals, typename Tbody>
	struct function_traits<method_def<Tret, Tname, TFormals, Tbody>> {
		using result_type = Tret;
		using arguments = typename method_def<Tret, Tname, TFormals, Tbody>::arguments;
	};

	///////////////////////////////////////////////////////////////////////////////

	template<typename T>
	struct bases {
		using type = T;
		template<template<typename, int> class F = EmptyBinder, int Nesting = 0>
		using bind = typename F<bases<T>, Nesting>::type;
	};

	///////////////////////////////////////////////////////////////////////////////

	template<typename... Ts>
	struct members : named_elements<members, Ts...> {};

	///////////////////////////////////////////////////////////////////////////////

	template<typename... Ts>
	struct methods : named_elements<methods, Ts...> {};

	///////////////////////////////////////////////////////////////////////////////

	template<typename Name, typename Bases, typename Members, typename Methods>
	struct class_def : Members::types, Bases::type {
		using type = class_def<Name, Bases, Members, Methods>;
		using name = Name;

		using members = Members;
		using methods = Methods;

		template<int I>
		using method = std::tuple_element<I, typename Methods::all>;

		template<template<typename, int> class F, int Nesting>
		struct bind_helper {
			template<typename T, int Nesting, int index>
			using MemberIndexWrapper = member_index<T, index>;
			template<typename T, int N>
			using BoundNameBinder = name_binder<T, Members, MemberIndexWrapper, F, Nesting>;
			//Binding applies only to stmts
			//TODO: also bind on members of base classes
			using type = typename F<class_def<typename Name::template bind<F, Nesting>, typename Bases::template bind<F, Nesting>, Members, typename Methods::template bind<BoundNameBinder, Nesting>>, Nesting>::type;
		};
		
		template<template<typename, int> class F = EmptyBinder, int Nesting = 0>
		using bind = typename bind_helper<F, Nesting + 1>::type;
	};
}

#endif