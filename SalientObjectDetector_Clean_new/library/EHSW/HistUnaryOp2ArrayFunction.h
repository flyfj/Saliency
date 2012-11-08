#pragma once

#include "BinUnaryOp2ArrayFunction.h"

// function instances
//template<class RealType> class HistLinearSVMFunc;
//template<class RealType> class HistDotProductFastFunc;
//template<class RealType> class HistBhattachSimilarityFastFunc;
//template<class RealType> class HistUnaryUniquenessFunc;
//template<class RealType> class HistUnaryCommonnessFunc;

//////////////////////////////////////////////////////////////////////////
template<class BinOp, bool UseResidual = false, class SumOp = unary_identity<typename BinOp::result_type>, class BinIndexType = unsigned int, class BinCountType = unsigned int>
class HistUnaryOp2ArrayFunction : public HistUnaryFunction<BinUnaryOp2ArrayFunction<BinOp, UseResidual, BinIndexType, BinCountType>, SumOp>
{
	typedef HistUnaryFunction<BinUnaryOp2ArrayFunction<BinOp, UseResidual, BinIndexType, BinCountType>, SumOp> Base;

public:
	template<class VectorType>
	HistUnaryOp2ArrayFunction(const VectorType& _op2_array, const BinOp& bin_op = BinOp(), const SumOp& s_op = SumOp()) 
		: Base(_op2_array.size(), bin_fun_type(_op2_array, bin_op), s_op) {}
};

// linear svm function : f = invN * (\sum_b w_b * h_b) - offset
template<class _Ty>
struct linear_svm_normalizer : public unary_function<_Ty, _Ty>
{		
	_Ty normalizer, offset;
	linear_svm_normalizer(const _Ty& n, const _Ty& o) : normalizer(n), offset(o){}
	_Ty operator()(const _Ty& _Left) const	{ return normalizer*_Left - offset;	}
};

template<class RealType>
class HistLinearSVMFunc : public HistUnaryOp2ArrayFunction<binary_dot<RealType>, false, linear_svm_normalizer<RealType>>
{
	typedef HistUnaryOp2ArrayFunction<binary_dot<RealType>, false, linear_svm_normalizer<RealType>> Base;

public:
	static string Name() {	return "SVM_Linear";	}

	template<class VectorType>
	HistLinearSVMFunc(const VectorType& weights, unsigned int TotalCount, const RealType& offset) 
		: Base(weights, binary_dot<RealType>(), linear_svm_normalizer<RealType>(1.0f/(RealType)TotalCount, offset)){}

	void SetTotalCount(unsigned int TotalCount)
	{
		sum_op.normalizer = 1.0f/(RealType)TotalCount;
	}
};

template<class RealType>
class HistDotProductFastFunc : public HistUnaryOp2ArrayFunction<binary_dot<RealType>, false, unary_multiply<RealType>>
{
	typedef HistUnaryOp2ArrayFunction<binary_dot<RealType>, false, unary_multiply<RealType>> Base;

public:
	static string Name() {	return bin_fun_type::Name() + "Fast";	}

	template<class VectorType>
	HistDotProductFastFunc(const VectorType& tar_hist, unsigned int TotalCount) 
		: Base(tar_hist, binary_dot<RealType>(), unary_multiply<RealType>(1.0f/(RealType)TotalCount)){}

	void SetTotalCount(unsigned int TotalCount)
	{			
		sum_op = unary_multiply<RealType>(1.0f/(RealType)TotalCount);
	}
};

template<class RealType>
class HistBhattachSimilarityFastFunc : public HistUnaryOp2ArrayFunction<binary_sqrt_multiply<RealType>, false, unary_multiply<RealType>>
{
	typedef HistUnaryOp2ArrayFunction<binary_sqrt_multiply<RealType>, false, unary_multiply<RealType>> Base;

public:
	static string Name() {	return bin_fun_type::Name() + "Fast";	}

	template<class VectorType>
	HistBhattachSimilarityFastFunc(const VectorType& tar_hist, unsigned int TotalCount) 
		: Base(tar_hist, binary_sqrt_multiply<RealType>(), unary_multiply<RealType>(1.0f/sqrt((RealType)TotalCount))){}

	void SetTotalCount(unsigned int TotalCount)	
	{
		sum_op = unary_multiply<RealType>(1.0f/sqrt((RealType)TotalCount));
	}
};

// the number of bins in histogram 1 that can't be represented by those in histogram 2 (unique bins)
template<class RealType>
class HistUnaryUniquenessFunc : public HistUnaryOp2ArrayFunction<binary_max_of_diff_and_zero<unsigned int>, true, unary_multiply<RealType>>
{
	typedef HistUnaryOp2ArrayFunction<binary_max_of_diff_and_zero<unsigned int>, true, unary_multiply<RealType>> Base;

public:
	template<class VectorType>
	HistUnaryUniquenessFunc(const VectorType& _op2_array, unsigned int TotalCount) 
		: Base(_op2_array, binary_max_of_diff_and_zero<unsigned int>(), unary_multiply<RealType>(1.0f/(RealType)TotalCount)) {}

 	void SetTotalCount(unsigned int TotalCount)
 	{	
 		sum_op = unary_multiply<RealType>(1.0f/(RealType)TotalCount);
 	}
};

// the number of bins in histogram 1 that can be represented by those in histogram 2 (common bins)
template<class RealType>
class HistUnaryCommonnessFunc : public HistUnaryOp2ArrayFunction<binary_min<unsigned int>, true, unary_multiply<RealType>>
{
	typedef HistUnaryOp2ArrayFunction<binary_min<unsigned int>, true, unary_multiply<RealType>> Base;

public:
	template<class VectorType>
	HistUnaryCommonnessFunc(const VectorType& _op2_array, unsigned int TotalCount) 
		: Base(_op2_array, binary_min<unsigned int>(), unary_multiply<RealType>(1.0f/(RealType)TotalCount)) {}

	void SetTotalCount(unsigned int TotalCount)
	{  
		sum_op = unary_multiply<RealType>(1.0f/(RealType)TotalCount);
	}
};
