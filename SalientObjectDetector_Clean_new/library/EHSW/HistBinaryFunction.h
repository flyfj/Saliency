#pragma once

#include <fast_histogram.h>

#include <assert.h>
using namespace std;

#include "BinAdditiveFunction.h"

// function instances
//template<class RealType> class HistBinaryNullFunction;

//////////////////////////////////////////////////////////////////////////
// bin binary function that only depends on the count but not the index
// BinOp is derived from binary_function
// if UseCenterSurround = true, the first histogram is surround and the second is center
template<class BinOp, bool UseCenterSurround, class BinIndexType = unsigned int, class BinCountType = unsigned int>
class BinCountBinaryFunction : public BinFunctionBase<typename BinOp::result_type, BinIndexType, BinCountType>
{
protected:
	BinOp bin_op;

public:
	// as it is a constant, the condition on it should be removed by compiler
	enum { USE_CENTER_SURROUND = UseCenterSurround };

	BinCountBinaryFunction(const BinOp& op = BinOp()) : bin_op(op) {}

	bin_result_type operator()(const bin_index_type& b, const bin_count_type& c1, const bin_count_type& c2) const	
	{
		if (USE_CENTER_SURROUND)
		{
			assert(c1 >= c2);
			return bin_op(c1-c2, c2);
		}
		else
		{
			return bin_op(c1, c2);
		}
	}
};

// A bin additive function that takes two histograms as input, f(H1, H2) = \sum_b op(H1_b, H2_b)
// BinFunction should be derived from BinBinaryFunctionBase
template<class BinFunction, class SumOp = unary_identity<typename BinFunction::bin_result_type>>
class HistBinaryFunction : public BinAdditiveFunction<BinFunction, SumOp>
{
	typedef BinAdditiveFunction<BinFunction, SumOp> Base;	

public:
	HistBinaryFunction(unsigned int hist_length, const BinFunction& b_fun, const SumOp& s_op = SumOp())	: Base(hist_length, b_fun, s_op) {}

	template<class Hist1Type, class Hist2Type>
	fun_result_type InitEvaluate(const Hist1Type& hist1, const Hist2Type& hist2)	
	{	
		assert(bin_results.size() == hist1.Length());
		assert(bin_results.size() == hist2.Length());

		numOfUpdateBinCalls = 0;
		sum_bin_result = 0;

		for(unsigned int b = 0; b < bin_results.size(); b++)
			sum_bin_result += (bin_results[b] = bin_fun(b, hist1[b], hist2[b]));
		return sum_op(sum_bin_result);
	}	

	template<int OperandNumber>
	void UpdateBin(const unsigned int& b, const int& delta, HistArray& hist1, HistArray& hist2)	{}	// should use static assert to avoid general case

	template<>
	void UpdateBin<1>(const unsigned int& b, const int& delta, HistArray& hist1, HistArray& hist2)	
	{
		UpdateBinResult(b, bin_fun(b, hist1[b]+=delta, hist2[b]));
	}

	template<>
	void UpdateBin<2>(const unsigned int& b, const int& delta, HistArray& hist1, HistArray& hist2)
	{
		UpdateBinResult(b, bin_fun(b, hist1[b], hist2[b]+=delta));
	}
};

template<class RealType>
class HistBinaryNullFunction : public HistBinaryFunction<BinCountBinaryFunction<binary_null<RealType>, true>>
{
public:	
	HistBinaryNullFunction(unsigned int hist_length) 
		: HistBinaryFunction<BinCountBinaryFunction<binary_null<RealType>, true>>(hist_length, BinCountBinaryFunction<binary_null<RealType>, true>()){}
};

// the number of bins in center histogram that can't be represented by those in surround histogram (unique bins)
//*
template<class RealType>
class HistCenterSurroundUniquenessFunc : public HistBinaryFunction<BinCountBinaryFunction<binary_max_of_diff_and_zero<unsigned int>, true>, unary_multiply<RealType>>
{
	typedef HistBinaryFunction<BinCountBinaryFunction<binary_max_of_diff_and_zero<unsigned int>, true>, unary_multiply<RealType>> Base;

public:
	HistCenterSurroundUniquenessFunc(unsigned int hist_length, unsigned int TotalCount1, unsigned int TotalCount2) 
		: Base(hist_length, BinCountBinaryFunction<binary_max_of_diff_and_zero<unsigned int>, true>(), unary_multiply<RealType>(1.0f/(RealType)TotalCount2))
	{}
};
//*/