#ifndef METAAST_EXPRESSIONS_HPP
#define METAAST_EXPRESSIONS_HPP

namespace metaast {

	#define IMPL_UNARY_EXPR_EX(_name, _pre, _post)													\
		template <typename T>																		\
		struct _name {																				\
			template<template<typename, int> class F = EmptyBinder, int Nesting = 0>				\
			using bind = typename F<_name<typename T::template bind<F, Nesting>>, Nesting>::type;	\
			template <typename... Tsts>																\
			auto operator()(Tsts... sts) const -> decltype(_pre T()(sts...) _post)					\
				{ return _pre T()(sts...) _post; }													\
		}

	#define METAAST_EMPTY
	#define IMPL_UNARY_EXPR(_name, _op)	IMPL_UNARY_EXPR_EX(_name, _op, METAAST_EMPTY)
	#define IMPL_PREFIX_INCREMENT_DECREMENT_EXPR(_name, _op) IMPL_UNARY_EXPR(_name, _op)
	#define IMPL_POSTFIX_INCREMENT_DECREMENT_EXPR(_name, _op) IMPL_UNARY_EXPR_EX(_name, METAAST_EMPTY, _op)

	//unary ++ --
	IMPL_UNARY_EXPR(uplus,			+);
	IMPL_UNARY_EXPR(uminus,			-);
	IMPL_UNARY_EXPR(logical_not,	!);
	IMPL_UNARY_EXPR(complement,		~);
	IMPL_UNARY_EXPR(addressof,		&);
	IMPL_UNARY_EXPR(dereference,	*);

	IMPL_PREFIX_INCREMENT_DECREMENT_EXPR(pre_inc,	++);
	IMPL_PREFIX_INCREMENT_DECREMENT_EXPR(pre_dec,	--);
	IMPL_POSTFIX_INCREMENT_DECREMENT_EXPR(post_inc,	++);
	IMPL_POSTFIX_INCREMENT_DECREMENT_EXPR(post_dec,	--);

	#undef IMPL_POSTFIX_INCREMENT_DECREMENT_EXPR
	#undef IMPL_PREFIX_INCREMENT_DECREMENT_EXPR
	#undef IMPL_UNARY_EXPR
	#undef METAAST_EMPTY
	#undef IMPL_UNARY_EXPR_EX

	///////////////////////////////////////////////////////////////////////////////

	template <typename T>
	struct parenthesis {
		template<template<typename, int> class F = EmptyBinder, int Nesting = 0>
		using bind = typename F<parenthesis<typename T::template bind<F, Nesting>>, Nesting>::type;
		template <typename... Tsts>
		auto operator()(Tsts... sts) const -> decltype(T()(sts...)) { return T()(sts...); }
	};

	template <typename T, typename Type>
	struct cast {
		template<template<typename, int> class F = EmptyBinder, int Nesting = 0>
		using bind = typename F<cast<typename T::template bind<F, Nesting>, Type>, Nesting>::type;
		template <typename... Tsts>
		Type operator()(Tsts... sts) const { return (Type)(T()(sts...)); }
	};

	///////////////////////////////////////////////////////////////////////////////

	#define IMPL_BINARY_EXPR(_name, _op)														\
		template <typename Tleft, typename Tright>												\
		struct _name {																			\
			template<template<typename, int> class F = EmptyBinder, int Nesting = 0>			\
			using bind = typename F<_name<														\
				typename Tleft::template bind<F, Nesting>,										\
				typename Tright::template bind<F, Nesting>										\
			>, Nesting>::type;																	\
			template <typename... Tsts>															\
			auto operator()(Tsts... sts) const -> decltype(Tleft()(sts...) _op Tright()(sts...))\
				{ return Tleft()(sts...) _op Tright()(sts...); }								\
		}

	//arithmetic
	IMPL_BINARY_EXPR(add,			+);
	IMPL_BINARY_EXPR(sub,			-);
	IMPL_BINARY_EXPR(mul,			*);
	IMPL_BINARY_EXPR(div,			/);
	IMPL_BINARY_EXPR(mod,			%);

	//shifts
	IMPL_BINARY_EXPR(lshift,		<<);
	IMPL_BINARY_EXPR(rshift,		>>);

	//relational/equality
	IMPL_BINARY_EXPR(less,			<);
	IMPL_BINARY_EXPR(less_equal,	<=);
	IMPL_BINARY_EXPR(greater,		>);
	IMPL_BINARY_EXPR(greater_equal, >=);
	IMPL_BINARY_EXPR(equal,			==);
	IMPL_BINARY_EXPR(not_equal,		!=);

	//binary
	IMPL_BINARY_EXPR(binary_and,	&);
	IMPL_BINARY_EXPR(binary_or,		|);
	IMPL_BINARY_EXPR(binary_xor,	^);

	//logical
	IMPL_BINARY_EXPR(logical_and,	&&);
	IMPL_BINARY_EXPR(logical_or,	||);

	//assignments
	IMPL_BINARY_EXPR(assign,		=);
	IMPL_BINARY_EXPR(add_assign,	+=);
	IMPL_BINARY_EXPR(sub_assign,	-=);
	IMPL_BINARY_EXPR(mul_assign,	*=);
	IMPL_BINARY_EXPR(div_assign,	/=);
	IMPL_BINARY_EXPR(mod_assign,	%=);
	IMPL_BINARY_EXPR(lshift_assign,	<<=);
	IMPL_BINARY_EXPR(rshift_assign,	>>=);
	IMPL_BINARY_EXPR(and_assign,	&=);
	IMPL_BINARY_EXPR(or_assign,		|=);
	IMPL_BINARY_EXPR(xor_assign,	^=);

	//others
	#define METAAST_COMMA ,
	IMPL_BINARY_EXPR(comma,	METAAST_COMMA);
	#undef METAAST_COMMA
	
	//TODO: [] pointer to member

	#undef IMPL_BINARY_EXPR

	///////////////////////////////////////////////////////////////////////////////

	template <typename Tcond, typename T1, typename T2>
	struct ternary_expr {
		template<template<typename, int> class F = EmptyBinder, int Nesting = 0>
		using bind = typename F<ternary_expr<
			typename Tcond::template bind<F, Nesting>,
			typename T1::template bind<F, Nesting>,
			typename T2::template bind<F, Nesting>
		>, Nesting>::type;
		template <typename... Tsts>
		auto operator()(Tsts... sts) const -> decltype(Tcond()(sts...) ? T1()(sts...) : T2()(sts...))
			{ return Tcond()(sts...) ? T1()(sts...) : T2()(sts...); }
	};
}

#endif
