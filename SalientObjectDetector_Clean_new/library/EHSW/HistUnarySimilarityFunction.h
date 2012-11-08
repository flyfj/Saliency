//////////////////////////////////////////////////////////////////////////
// histogram functions to compute similarity with a target histogram

#pragma once

#include "BinUnaryOp2ArrayFunction.h"

// function instances
//class HistIntersectionFunc;
//class HistBhattachSimilarityFunc;
//class HistDotProductFunc;

//class HistL1NormSimilarityFunc;
//class HistL2NormSimilarityFunc;
//class HistChiSquareSimilarityFunc;

//class HistIntersectionResidualFunc;

//////////////////////////////////////////////////////////////////////////
template<class BinOp, class SumOp = unary_identity<typename BinOp::result_type>, class BinIndexType = unsigned int, class BinCountType = unsigned int>
class HistUnarySimilarityFunc : public HistUnaryFunction<BinUnaryOp2ArrayNorm1Function<BinOp, false, BinIndexType, BinCountType>, SumOp>
{	
	typedef HistUnaryFunction<BinUnaryOp2ArrayNorm1Function<BinOp, false, BinIndexType, BinCountType>, SumOp> Base;

public:
	static string Name() {	return bin_fun_type::Name();	}

	template<class VectorType>
	HistUnarySimilarityFunc(const VectorType& tar_hist, unsigned int TotalCount, const BinOp& op = BinOp(), const SumOp& s_op = SumOp())
		: Base(tar_hist.size(), bin_fun_type(tar_hist, 1.0f/(typename bin_fun_type::real_type)TotalCount, op), s_op)	{}

	void SetTotalCount(unsigned int TotalCount)
	{
		bin_fun.Normalizer = 1.0f/(typename bin_fun_type::real_type)TotalCount;
	}

	// just for performance test, as the op2_array is not specified
	void SetHistLength(unsigned int hist_length)
	{
		Base::SetHistLength(hist_length);
		bin_fun.SetHistLength(hist_length);
	}
};

#define DECLARE_BIN_SIM_HIST_FUN(FUN_CLASS_NAME, OP_CLASS_NAME, SUM_OP_NAME, REAL_TYPE) /* define a specific HistUnarySimilarityFunc */ \
	typedef HistUnarySimilarityFunc< OP_CLASS_NAME<REAL_TYPE>, SUM_OP_NAME<REAL_TYPE> > FUN_CLASS_NAME;

DECLARE_BIN_SIM_HIST_FUN(HistIntersectionFunc, binary_min, unary_identity, float)
DECLARE_BIN_SIM_HIST_FUN(HistBhattachSimilarityFunc, binary_sqrt_multiply, unary_identity, float)
DECLARE_BIN_SIM_HIST_FUN(HistDotProductFunc, binary_dot, unary_identity, float)

// mapping L1 distance [0..2] to similarity [1..0]
template<class _Ty>
struct linear_mapping_02_to_10 : public unary_function<_Ty, _Ty>
{		
	_Ty operator()(const _Ty& _Left) const	{	return 1.0f - 0.5f*_Left;	}
};
DECLARE_BIN_SIM_HIST_FUN(HistL1NormSimilarityFunc, binary_abs, linear_mapping_02_to_10, float)

// mapping L2 distance [0..sqrt(2)] to similarity [1..0]
template<class _Ty>
struct linear_mapping_0sqr2_to_10 : public unary_function<_Ty, _Ty>
{		
	_Ty operator()(const _Ty& _Left) const	{	return 1.0f - sqrt(0.5f*_Left);	}
};
DECLARE_BIN_SIM_HIST_FUN(HistL2NormSimilarityFunc, binary_sqr_abs, linear_mapping_0sqr2_to_10, float)

// mapping chi-square distance [0..2] to similarity [1..0]
DECLARE_BIN_SIM_HIST_FUN(HistChiSquareSimilarityFunc, binary_chi_sqr, linear_mapping_02_to_10, float)

//////////////////////////////////////////////////////////////////////////
template<class BinOp, class SumOp = unary_identity<typename BinOp::result_type>, class BinIndexType = unsigned int, class BinCountType = unsigned int>
class HistUnaryResidualSimilarityFunc : public HistUnaryFunction<BinUnaryOp2ArrayNorm12Function<BinOp, true, BinIndexType, BinCountType>, SumOp>
{	
	typedef HistUnaryFunction<BinUnaryOp2ArrayNorm12Function<BinOp, true, BinIndexType, BinCountType>, SumOp> Base;

public:
	static string Name() {	return BinOp::Name();	}

	template<class VectorType>
	HistUnaryResidualSimilarityFunc(const VectorType& tar_hist, unsigned int TotalCount, unsigned int TotalCount2, const BinOp& op = BinOp(), const SumOp& s_op = SumOp())
		: Base(tar_hist.size(), bin_fun_type(tar_hist, 1.0f/(typename bin_fun_type::real_type)TotalCount, 1.0f/(typename bin_fun_type::real_type)(TotalCount2-TotalCount), op), s_op)	
	{
		// make sure the counting is correct
		assert(TotalCount2 > TotalCount);
		int sum = 0;
		for(unsigned int n = 0; n < tar_hist.size(); n++)
			sum += tar_hist[n];
		assert(sum == TotalCount2);
	}

	void SetTotalCount(unsigned int TotalCount, unsigned int TotalCount2)
	{
		assert(TotalCount2 > TotalCount);
		bin_fun.Normalizer = 1.0f/(typename bin_fun_type::real_type)TotalCount;
		bin_fun.Normalizer2 = 1.0f/(typename bin_fun_type::real_type)(TotalCount2 - TotalCount);
	}
};

#define DECLARE_BIN_RESIDUAL_SIM_HIST_FUN(FUN_CLASS_NAME, OP_CLASS_NAME, SUM_OP_NAME, REAL_TYPE) /* define a specific HistUnaryResidualSimilarityFunc */ \
	typedef HistUnaryResidualSimilarityFunc< OP_CLASS_NAME<REAL_TYPE>, SUM_OP_NAME<REAL_TYPE> > FUN_CLASS_NAME;

DECLARE_BIN_RESIDUAL_SIM_HIST_FUN(HistIntersectionResidualFunc, binary_min, unary_identity, float)

// straightforward computation of similarity of two histograms given a bin operator
/*
// BinSimOp should be derived from BinSimOpBase
template<class TargetHistType, class BinSimOp>
typename BinSimOp::fun_result_type HistSimilarity(const HistSparse& hist, const TargetHistType& targetHist, const BinSimOp& bin_op)
{		
	assert(hist.Length() == targetHist.size());
	typename BinSimOp::fun_result_type sum = 0;

	// cannot assume bin_op result is 0 when op1 is 0, it is not necessarily true
	if (BinSimOp::IS_0_WHEN_OP1_IS_0)
	{
		//const_cast<unsigned int&>(num_eval_sim)++;
		//const_cast<unsigned int&>(num_ops) += size();
		for(HistSparse::const_iterator it = hist.begin(); it != hist.end(); it++)
			sum += bin_op(static_cast<BinSimOp::bin_value_type>(it->second), targetHist[it->first]);
	}
	else 
	{	
		for(unsigned int b = 0; b < targetHist.size(); b++)
			sum += bin_op(static_cast<BinSimOp::bin_value_type>(hist[b]), targetHist[b]);
	}
	return bin_op.sum_op(sum);
}

template<class TargetHistType, class BinSimOp>
typename BinSimOp::fun_result_type HistSimilarity(const HistArray& hist, const TargetHistType& targetHist, const BinSimOp& bin_op)
{
	assert(hist.Length() == targetHist.size());
	typename BinSimOp::fun_result_type sum = 0;
	for(unsigned int b = 0; b < hist.Length(); b++)
		sum += bin_op(static_cast<BinSimOp::bin_value_type>(hist[b]), targetHist[b]);
	return bin_op.sum_op(sum);
}
//*/