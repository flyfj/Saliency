#pragma once

#include "HistBinaryFunction.h"

// function instances
//class HistCenterSurrIntersectionFunc;
//class HistBinaryIntersectionFunc;

//////////////////////////////////////////////////////////////////////////
template<class BinOp, bool UseCenterSurround, class BinIndexType = unsigned int, class BinCountType = unsigned int>
class BinSimilarityBinaryFunction : public BinCountBinaryFunction<BinOp, UseCenterSurround, BinIndexType, BinCountType>
{
	typedef BinCountBinaryFunction<BinOp, UseCenterSurround, BinIndexType, BinCountType> Base;
public:
	typedef bin_result_type real_type;

	// normalizing constants for two operands
	real_type Normalizer1, Normalizer2;

	BinSimilarityBinaryFunction(const BinOp& op, unsigned int TotalCount1, unsigned int TotalCount2) : Base(op)
	{
		SetTotalCount(TotalCount1, TotalCount2);
	}

	void SetTotalCount(unsigned int TotalCount1, unsigned int TotalCount2) 
	{
		if (USE_CENTER_SURROUND)
		{
			assert(TotalCount1 > TotalCount2);
			Normalizer1 = (real_type(1)/real_type(TotalCount1-TotalCount2));
		} 
		else
		{
			Normalizer1 = (real_type(1)/real_type(TotalCount1));
		}
		
		Normalizer2 = real_type(1)/real_type(TotalCount2);
	}

	bin_result_type operator()(const bin_index_type& b, const bin_count_type& c1, const bin_count_type& c2) const
	{	
		if (USE_CENTER_SURROUND)
		{
			assert(c1 >= c2);
			return bin_op((c1-c2)*Normalizer1, c2*Normalizer2);
		}
		else
		{
			return bin_op(c1*Normalizer1, c2*Normalizer2);
		}
	}
};

template<class BinOp, bool UseCenterSurround, class SumOp = unary_identity<typename BinOp::result_type>, class BinIndexType = unsigned int, class BinCountType = unsigned int>
class HistBinarySimilarityFunction : public HistBinaryFunction<BinSimilarityBinaryFunction<BinOp, UseCenterSurround, BinIndexType, BinCountType>, SumOp>
{
	typedef HistBinaryFunction<BinSimilarityBinaryFunction<BinOp, UseCenterSurround, BinIndexType, BinCountType>, SumOp> Base;

public:
	HistBinarySimilarityFunction(unsigned int hist_length, unsigned int TotalCount1, unsigned int TotalCount2, const BinOp& op = BinOp(), const SumOp& s_op = SumOp())
		: Base(hist_length, bin_fun_type(op, TotalCount1, TotalCount2), s_op) {}
};

#define DECLARE_HIST_BINARY_SIM_FUN(FUN_CLASS_NAME, OP_CLASS_NAME, UseCenterSurround, SUM_OP_NAME, REAL_TYPE) /* define a specific HistBinaryFunction */ \
	typedef HistBinarySimilarityFunction<OP_CLASS_NAME<REAL_TYPE>, UseCenterSurround, SUM_OP_NAME<REAL_TYPE> > FUN_CLASS_NAME;

DECLARE_HIST_BINARY_SIM_FUN(HistCenterSurrIntersectionFunc, binary_min, true, unary_identity, float)
DECLARE_HIST_BINARY_SIM_FUN(HistBinaryIntersectionFunc, binary_min, false, unary_identity, float)