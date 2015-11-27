#include <metaast.hpp>

#include <cassert>
#include <functional>
#include <iostream>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>
#include <list>
#include <numeric>

///////////////////////////////////////////////////////////////////////////////

namespace metaast {
	
	/////////////////
	// Visitor Support
	//
	template<typename T, template<typename> class F>
	struct visit_empty { using type = typename F<T>::type; };

	template<typename T, template<typename> class F>
	struct visit : visit_empty<T, F> {};

	template<template<typename> class F, template<typename...> class Node, typename... Ts>
	struct visit_all { using type = typename F<Node<typename visit<Ts, F>::type...>>::type; };

	#define IMPL_VISIT_1(_name)									\
		template<typename T, template<typename> class F>		\
		struct visit<_name<T>, F> : visit_all<F, _name, T> {}

	#define IMPL_VISIT_2(_name)								\
		template<typename T1, typename T2, template<typename> class F>	\
		struct visit<_name<T1, T2>, F> : visit_all<F, _name, T1, T2> {}

	#define IMPL_VISIT_3(_name)														\
		template<typename T1, typename T2, typename T3, template<typename> class F>	\
		struct visit<_name<T1, T2, T3>, F> : visit_all<F, _name, T1, T2, T3> {}

	#define IMPL_VISIT_4(_name)																		\
		template<typename T1, typename T2, typename T3, typename T4, template<typename> class F>	\
		struct visit<_name<T1, T2, T3, T4>, F> : visit_all<F, _name, T1, T2, T3, T4> {}

	#define IMPL_VISIT_N(_name)											\
		template<template<typename> class F, typename... Ts>			\
		struct visit<_name<Ts...>, F> : visit_all<F, _name, Ts...> {}

	#define IMPL_VISIT_1_N(_name)											\
		template<template<typename> class F, typename T, typename... Ts>	\
		struct visit<_name<T, Ts...>, F> : visit_all<F, _name, T, Ts...> {}

	IMPL_VISIT_2(assign);
	IMPL_VISIT_2(add);
	IMPL_VISIT_2(sub);
	IMPL_VISIT_2(mul);
	IMPL_VISIT_2(div);
	IMPL_VISIT_2(mod);

	IMPL_VISIT_2(if_stmt);
	IMPL_VISIT_3(if_stmt);

	IMPL_VISIT_1(return_stmt);

	IMPL_VISIT_N(formals);
	IMPL_VISIT_N(locals);
	IMPL_VISIT_N(members);
	IMPL_VISIT_N(methods);
	IMPL_VISIT_4(func_def);

	IMPL_VISIT_1_N(call_expr);
	IMPL_VISIT_1_N(block);

	IMPL_VISIT_N(defs);
	IMPL_VISIT_2(program);

	#undef IMPL_VISIT_1
	#undef IMPL_VISIT_2
	#undef IMPL_VISIT_3
	#undef IMPL_VISIT_N
	#undef IMPL_VISIT_1_N

	template<typename T, template<typename> class F>
	using visit_t = typename visit<T, F>::type;

	/////////////////
	//Test
	//
	void print_str(std::string s) { std::cout << s << std::endl; }
	void print_int(int x) { std::cout << x << std::endl; }

	int foo(int x, int y){ std::cout << "Free function: "; return x + y; };
	struct Foo {
		int operator()(int x, int y) { std::cout << "Functor object: "; return x + y; }
	};

	static void test_blocks (void) {
		using f = func_def<
			int, NAME(f), formals<var<int, NAME(x)>, var<int, NAME(y)>>,
			block<
				locals<var<int, NAME(a), Int<10>>, var<int, NAME(b)>>,
				block<
					locals<var<int, NAME(a), Int<20>>>,
					assign<NAME(b), NAME(y)>
				>,
				return_stmt<
					add<NAME(a), NAME(b)>
				>
			>
		>::bind<>;
		
		std::cout << "result = " << call_expr<f, Int<0>, Int<1>>()() << std::endl;
		std::cout << "result = " << call_expr<Foo, Int<2>, Int<3>>()() << std::endl;
		std::cout << "result = " << call_expr<EXTERN_FUNC(foo), Int<4>, Int<5>>()() << std::endl;
	}

	void test_stmts(void) {
		using f = func_def<
			void, NAME(f), formals<var<int, NAME(n)>>,
			block<
				locals<var<int, NAME(i)>>,
				for_stmt<
					assign<NAME(i), Int<0>>,
					less<NAME(i), NAME(n)>,
					pre_inc<NAME(i)>,
					block<locals<>,
						if_stmt<equal<NAME(i), sub<NAME(n), Int<1>>>, break_stmt>,
						call_expr<EXTERN_FUNC(print_int), NAME(i)>
					>
				>
			>
		>::bind<>;
		std::cout << "test_stmts: " << std::endl;
		call_expr<f, Int<5>>()();
	}
	struct Point {
		int x, y;
		void f(int a, int b) { std::cout << "Point::f(" << a << ", " << b << ")" << std::endl; }
		Point(int x = 0, int y = 0) : x(x), y(y) {}
	};
	void test_class(void) {
		using p = external_var<Point>;
		using Class = class_def<
			NAME(P),
			bases<Point>,
			members<var<int, NAME(z)>>,
			methods<
				method_def<int, NAME(g), formals<var<int, NAME(x)>>,
					block<locals<>,
						call_expr<EXTERN_FUNC(print_int), NAME(x)>,
						return_stmt<NAME(z)>
					>
				>
			>
		>::bind<>;
		using prog = program<
			defs<
				func_def<int, NAME(f), formals<>,
				block<locals<p, var<Class, NAME(a)>>,
						assign<MEMBER(a, z), Int<10>>,
						assign<EXTERN_MEMBER(p, x), Int<20>>,
						call_expr<EXTERN_METHOD(p, f), Int<3>, Int<4>>,
						return_stmt<add<call_expr<METHOD(a, g), Int<5>>, EXTERN_MEMBER(p, x)>>
					>
				>
			>,
			call_expr<EXTERN_FUNC(print_int), call_expr<NAME(f)>>
		>::bind<>;
		prog()();
	}

	using List = std::list<int>;
	using Iter = List::const_iterator;
	void print_list(const List& l) { std::cout << "( "; for(auto val : l) std::cout << val << " "; std::cout << ") = "; }
	int accumulate(Iter begin, Iter end) { return std::accumulate(begin, end, 0); }
	void test_externals(void) {
		using l = external_var<List>;
		using f = func_def<int, NAME(f), formals<>,
			block<
				locals<l>,
				call_expr<OVERLOADED_METHOD(l, (void (List::*)(const int&)), push_back), Int<1>>,
				call_expr<OVERLOADED_METHOD(l, (void (List::*)(const int&)), push_back), Int<2>>,
				call_expr<OVERLOADED_METHOD(l, (void (List::*)(const int&)), push_back), Int<3>>,
				call_expr<EXTERN_FUNC(print_list), l>,
				return_stmt<call_expr<EXTERN_FUNC(accumulate),
					call_expr<OVERLOADED_METHOD(l, (Iter (List::*)() const), begin)>,
					call_expr<OVERLOADED_METHOD(l, (Iter (List::*)() const), end)>
				>>
			>
		>::bind<>;
		
		std::cout << "list test, accumulate";
		std::cout << call_expr<f>()() << std::endl;
	}

	void f(void) { std::cout << "void f(void) test passed" << std::endl; }
	void test_extern_void_void_func(void) {
		using prog = program<
			defs<func_def<int, NAME(f), formals<>, call_expr<EXTERN_FUNC(f)>>>,
			call_expr<NAME(f)>
		>::bind<>;
		prog()();
	}

	template<typename T, unsigned int N>
	struct ExpandPower { using type = mul<T, typename ExpandPower<T, N - 1>::type>; };
	template<typename T>
	struct ExpandPower<T, 0> { using type = Int<1>; };

	template<int N>
	struct MakePower {
		using type = typename func_def<int, NAME(m), formals<var<int, NAME(x)>>,
			return_stmt<typename ExpandPower<NAME(x), N>::type>
		>::type;
	};

	void test_power() {
		using power3 = MakePower<3>::type::bind<>;
		std::cout << "power test: 2^3 = " << power3()(2) << std::endl;
	}

	template<typename T>
	struct AddLogging {
		template<typename T>
		struct make_func_def {
			using type = func_def<typename T::result_type, typename T::name, typename T::formals_type,
				block<
				locals<>,
				call_expr<EXTERN_FUNC(print_str), String<'e', 'n', 't', 'e', 'r', 'i', 'n', 'g', ' ', 'f', 'u', 'n', 'c'>>,
				typename T::body_type,
				call_expr<EXTERN_FUNC(print_str), String<'e', 'x', 'i', 't', 'i', 'n', 'g', ' ', 'f', 'u', 'n', 'c'>>
				>
			>;
		};
		template<typename T>
		struct make_return_stmt {
			using type = block<
				//TODO: first get the return val, print and then return
				locals<>,
				call_expr<EXTERN_FUNC(print_str), String<'e', 'x', 'i', 't', 'i', 'n', 'g', ' ', 'f', 'u', 'n', 'c'>>,
				T
			>;
		};
		using type = typename boost::mpl::eval_if<
			is_func_def<T>,
			make_func_def<T>,
		    boost::mpl::eval_if<
				is_return_stmt<T>,
				make_return_stmt<T>,
				std::identity<T>
			>
		>::type;
	};

	void test_power_with_logging() {
		using power3 = visit_t<MakePower<3>::type, AddLogging>::bind<>;
		std::cout << "power with logging test:" << std::endl;
		std::cout << "2^3 = " << power3()(2) << std::endl;
	}
}

int main() {
	metaast::test_blocks();
	metaast::test_stmts();
	metaast::test_class();
	metaast::test_externals();
	metaast::test_extern_void_void_func();
	metaast::test_power();
	metaast::test_power_with_logging();
	return 0;
}


