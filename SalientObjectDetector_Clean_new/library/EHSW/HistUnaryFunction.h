#pragma once

#include "BinAdditiveFunction.h"

#include <fast_histogram.h>
#include <assert.h>

// function instances
//class HistUnaryNullFunction;

//////////////////////////////////////////////////////////////////////////
template<class BinResultType, class BinIndexType = unsigned int, class BinCountType = unsigned int>
class BinUnaryFunctionBase : public BinFunctionBase<BinResultType, BinIndexType, BinCountType>
{
public:
	bin_result_type operator()(const bin_index_type& b, const bin_count_type& c) const	{	return 0;	}
};

// A bin additive function that takes one histogram as input, f(H) = \sum_b op(H_b)
// BinFunction should be derived from BinUnaryFunctionBase
template<class BinFunction, class SumOp = unary_identity<typename BinFunction::bin_result_type>>
class HistUnaryFunction : public BinAdditiveFunction<BinFunction, SumOp>
{
private:
	typedef BinAdditiveFunction<BinFunction, SumOp> Base;

public:
	HistUnaryFunction(unsigned int hist_length, const BinFunction& b_fun, const SumOp& s_op = SumOp())
		: Base(hist_length, b_fun, s_op){}

	// as un-normalized bin count is used as input during incremental computation
	// appropriate normalization by the total count of histogram (win_width*win_height) could be necessary
	void SetTotalCount(unsigned int TotalCount) {}
	void UpdateBin(const bin_index_type& b, const bin_count_type& c)	{	UpdateBinResult(b, bin_fun(b, c));	}

	// direct computation
	template<class HistType>
	fun_result_type Evaluate(const HistType& hist) const
	{	
		assert(bin_results.size() == hist.Length());
		bin_result_type sum = 0;
		for(unsigned int b = 0; b < bin_results.size(); b++)
			sum += bin_fun(b, hist[b]);
		return sum_op(sum);
	}	
	
	template<class HistType>
	fun_result_type InitEvaluate(const HistType& hist)
	{
		assert(bin_results.size() == hist.Length());
		numOfUpdateBinCalls = 0;
		sum_bin_result = 0;
		for(unsigned int b = 0; b < bin_results.size(); b++)
			sum_bin_result += (bin_results[b] = bin_fun(b, hist[b]));
		return sum_op(sum_bin_result);
	}
	
	// wrapper that takes window as parameters
	// this is just for compilation as it is impossible to incrementally compute a function dependent on window
	template<class HistType>
	fun_result_type Evaluate(const HistType& hist
		,unsigned int win_x, unsigned int win_y, unsigned int win_width, unsigned int win_height) const
	{
		return Evaluate(hist);
	}

	/*
	// this specialization implementation using sparse representation for base histogram is rare
	template<>
	fun_result_type InitEvaluate(const HistSparse& hist)
	{
		assert(bin_results.size() == hist.Length());
		numOfUpdateBinCalls = 0;
		sum_bin_result = 0;

		// cannot assume bin_op result is 0 given a 0 op1, it is not necessarily true
		if (BinSimOp::IS_0_WHEN_OP1_IS_0)
		{
			op_result = 0.0f;	// reset op_result to allow multiple call
			for(HistSparse::const_iterator it = hist.begin(); it != hist.end(); it++)
				sum_bin_result += (op_result[it->first] = BinResult(it->first, it->second));
		}
		else
		{
			// no need to explicitly reset op_result since it is computed here
			for(unsigned int b = 0; b < op_result.size(); b++)
				sum_bin_result += (op_result[b] = BinResult(b, hist[b]));
		}
		return bin_op.sum_op(sum_bin_result);
	}
	//*/
};

// null function, useful for performance test
template<class RealType>
class HistUnaryNullFunction : public HistUnaryFunction<BinUnaryFunctionBase<RealType>>
{
public:	
	HistUnaryNullFunction(unsigned int hist_length) : HistUnaryFunction<BinUnaryFunctionBase<RealType>>(hist_length, BinUnaryFunctionBase<RealType>()){}
};