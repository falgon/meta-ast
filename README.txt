////////////////////////////////////////////////////////////
//                      meta-ast                          //
////////////////////////////////////////////////////////////

An AST library for generative template metaprogramming.

////////////////////////////////////////////////////////////

Build: Requires Boost C++ libraries.
Tested with Visual Studio 2015.

////////////////////////////////////////////////////////////

Examples:

1) Function & call code generation

void print_int(int i) { std::cout << i << std::endl; }
using func = func_def<
	void, NAME(f), formals<var<int, NAME(n)>>,
	block<
		locals<var<int, NAME(i)>>,
		for_stmt<
			assign<NAME(i), Int<0>>,
			less<NAME(i), NAME(n)>,
			pre_inc<NAME(i)>,
				call_expr<EXTERN_FUNC(print_int), NAME(i)>
		>
	>
>::bind<>;
//func holds the AST for the function definition
//	void f(int n) {
//		int i;
//		for (i = 0; i < n; ++i)
//			print_int(i);
//	}

using code = call_expr<func, Int<5>>;
//code holds the AST for the function call
// f(5);

void test() { code()(); } //generate code here
//operates as if the original code was:
//	void test() { f(5); }
//with f defined as shown above

//**********************************************************

2) Specialized power versions (staged power multi-stage programming example)

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
	auto power3 = MakePower<3>::type::bind<>(); //generate code here
	auto power4 = MakePower<4>::type::bind<>();	//generate code here
	
	//generated code operates as if the orginal code was:
	//auto power3 = [](int x) { return x*x*x; };
	//auto power4 = [](int x) { return x*x*x*x; };
	//without involving for loops in the calculation of the powers
	
	std::cout << "power test: 2^3 = " << power3(2) << std::endl;	//8
	std::cout << "power test: 2^4 = " << power4(2) << std::endl;	//16
}
