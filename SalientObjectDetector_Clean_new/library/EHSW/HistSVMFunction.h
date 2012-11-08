#pragma once

#include "HistUnaryFunction.h"
#include "BinSimilarityOperator.h"

template<class RealType> class HistIntersectionSVMFunc;
template<class RealType> class HistChiSquareSVMFunc;

// general non-linear bin-wise svm kernel functions
template<class BinSimOp, class BinIndexType = unsigned int>	// BinSimOp should be derived from binary_function
class BinSVMFunction : public BinUnaryFunctionBase<typename BinSimOp::result_type, BinIndexType, typename BinSimOp::bin_count_type>
{	
public:	
	typedef typename BinSimOp::second_argument_type operand2_type;
	static string Name() {	return BinSimOp::Name();	}

	BinSimOp bin_op;

	BinSVMFunction(const vector<vector<operand2_type>>& sv
		,const vector<vector<operand2_type>>& weights
		,const BinSimOp& op) : svm_weights(weights), op2_arrays(sv), bin_op(op)	{}

	bin_result_type operator()(const bin_index_type& b, const bin_count_type& c) const	
	{
		bin_result_type sum = 0;
		const vector<operand2_type>& op2_array = op2_arrays[b];
		const vector<operand2_type>& weight = svm_weights[b];

		for(unsigned int n = 0; n < op2_array.size(); n++)
			sum += weight[n] * bin_op(c, op2_array[n]);

		return sum;
	}

	// just for performance test, as the svm weight and op2_arrays is not specified
	void SetHistLength(unsigned int hist_length)
	{		
		unsigned int num_sv = op2_arrays[0].size();
		op2_arrays.resize(hist_length, vector<operand2_type>(num_sv));
		svm_weights.resize(hist_length, vector<operand2_type>(num_sv));
		for(unsigned int b = 0; b < op2_arrays.size(); b++)
			for(unsigned int v = 0; v < op2_arrays[b].size(); v++)
			{
				op2_arrays[b][v] = (rand() % 10000) / 10000.f;
				svm_weights[b][v] = (rand() % 10000) / 10000.f;
			}
	}	

private:	
	// for each bin, we have a set of operands from the support vectors and svm weights	
	vector<vector<operand2_type>> op2_arrays, svm_weights;
};

// histogram function is weighted summation of kernel distances from the histogram to multiple vectors, e.g., support vectors in SVM
template<class BinSimOp, class BinIndexType = unsigned int>
class BinSumHistSVMFunc : public HistUnaryFunction<BinSVMFunction<BinSimOp, BinIndexType>, binder2nd<minus<typename BinSimOp::result_type>>>
{
	typedef BinSVMFunction<BinSimOp, BinIndexType> bin_svm_fun_type;
	typedef HistUnaryFunction<bin_svm_fun_type, binder2nd<minus<typename BinSimOp::result_type>>> Base;

public:
	typedef typename bin_svm_fun_type::operand2_type operand2_type;
	static string Name() {	return string("SVM") + "_" + BinSimOp::Name();	}

	BinSumHistSVMFunc(const vector<vector<operand2_type>>& sv
		,const vector<vector<operand2_type>>& weights		
		,const BinSimOp& op
		,const typename BinSimOp::result_type& offset) 
		: Base(sv.size(), bin_svm_fun_type(sv, weights, op), binder2nd<minus<typename BinSimOp::result_type>>(minus<typename BinSimOp::result_type>(), offset)) {}

	// just for performance test, as the op2_array is not specified
	void SetHistLength(unsigned int hist_length)
	{
		Base::SetHistLength(hist_length);
		bin_fun.SetHistLength(hist_length);
	}	

	void SetTotalCount(unsigned int TotalCount)
	{
		bin_fun.SetTotalCount(TotalCount);
	}
};

template<class RealType>
class HistIntersectionSVMFunc : public BinSumHistSVMFunc<IntersectionOperator<RealType>>
{
	typedef BinSumHistSVMFunc<IntersectionOperator<RealType>> Base;

public:
	HistIntersectionSVMFunc(const vector<vector<operand2_type>>& sv
		,const vector<vector<operand2_type>>& weights
		,unsigned int TotalCount
		,const RealType& offset) : Base(sv, weights, IntersectionOperator<RealType>(TotalCount), offset){}
};

// chi-square distance to similarity
template<class RealType>
struct ChiSquareSimilarityOperator : public ChiSquareDistanceOperator<RealType>
{
	typedef ChiSquareDistanceOperator<RealType> Base;
	static string Name() {	return "ChiSquareSim";	}

	explicit ChiSquareSimilarityOperator(int TotalCount) : Base(TotalCount){}

	RealType operator()(const bin_count_type& b1, const RealType& b2) const
	{			
		return -Base::operator()(b1, b2);
	}
};

template<class RealType>
class HistChiSquareSVMFunc : public BinSumHistSVMFunc<ChiSquareSimilarityOperator<RealType>>
{
	typedef BinSumHistSVMFunc<ChiSquareSimilarityOperator<RealType>> Base;

public:
	HistChiSquareSVMFunc(const vector<vector<operand2_type>>& sv
		,const vector<vector<operand2_type>>& weights
		,unsigned int TotalCount
		,const RealType& offset) : Base(sv, weights, ChiSquareSimilarityOperator<RealType>(TotalCount), offset){}
};