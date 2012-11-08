#pragma once

#include <valarray>
using namespace std;

#include <functional_ex.h>

// BinFunctionBase can be declared a virtual base class
// here we use template for efficiency but all such functions are virtual conceptually
template<class BinResultType, class BinIndexType = unsigned int, class BinCountType = unsigned int>
class BinFunctionBase
{
public:
	// should assert that both bin_index_type and bin_count_type are unsigned integral type
	typedef BinIndexType bin_index_type;
	typedef BinCountType bin_count_type;
	typedef BinResultType   bin_result_type;

	// for unary function, a trivial null implementation
	//bin_result_type operator()(const bin_index_type& b, const bin_count_type& c) const	{	return 0;	}

	// for binary function, a trivial null implementation
	//bin_result_type operator()(const bin_index_type& b, const bin_count_type& c1, const bin_count_type& c2) const	{	return 0;	}

	// return false if the bin is not contributing to the function, used to avoid unnecessary computation on useless bins
	// e.g., a sparse histogram index image where only certain interest pixels contribute
	bool IsBinUseful(const bin_index_type& b) const	{	return true;	}
};

// a bin additive function is sum of individual bin function results and therefore can be incrementally computed
// it is uniquely defined by a BinFunction and a SumOp (derived from unary_function)
template<class BinFunction, class SumOp = unary_identity<typename BinFunction::bin_result_type>>
class BinAdditiveFunction
{
public:	
	// should assert that both bin_index_type and bin_count_type are unsigned integral type
	typedef BinFunction bin_fun_type;
	typedef typename BinFunction::bin_index_type bin_index_type;
	typedef typename BinFunction::bin_count_type bin_count_type;
	typedef typename BinFunction::bin_result_type bin_result_type;

	typedef typename SumOp bin_sum_op_type;
	typedef typename SumOp::result_type fun_result_type;

	BinAdditiveFunction(unsigned int hist_length, const BinFunction& b_fun, const SumOp& s_op = SumOp())
		: bin_results(hist_length), sum_bin_result(0), bin_fun(b_fun), sum_op(s_op), numOfUpdateBinCalls(0){}

	bool IsBinUseful(const bin_index_type& b) const	{	return bin_fun.IsBinUseful(b);	}

	unsigned int HistLength() const {	return bin_results.size();	}
	void SetHistLength(unsigned int hist_length)
	{
		bin_results.resize(hist_length);
		sum_bin_result = 0;
	}

	fun_result_type EvaluateAfterUpdate()	{	return sum_op(sum_bin_result) ;	}

	unsigned int numOfUpdateBinCalls;	// count of UpdateBinResult() calls, used to compute histogram sparseness

protected:
	valarray<bin_result_type> bin_results;	// bin function results
	bin_result_type sum_bin_result;			// sum of bin function results
	SumOp sum_op;							// final result calculator

	// value copy of a bin function may be complex(e.g., svm functions)
	// however, use reference instead would require an non-temporary BinFunction object and this could be too demanding	
	bin_fun_type bin_fun;

	void UpdateBinResult(const bin_index_type& b, const bin_result_type& new_result)
	{
		numOfUpdateBinCalls++;

		bin_result_type& old_result = bin_results[b];
		sum_bin_result += new_result - old_result;
		old_result = new_result;
	}
};