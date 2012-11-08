#pragma once

#include "HistUnaryFunction.h"

// BinUnaryOp2ArrayFunction adopts a binary operator and an array of the same length of the histogram h
// the operator uses bin count as the first argument and the array provides the second argument
// BinOp should be derived from binary_function
// if UseResidual = false, the array is usually a normalized histogram used to compute a similarity with h
// if UseResidual = true, we actually use (array-h) as second argument
// therefore input array should be unnormalized because it should always contain h, e.g., computed from entire image
template<class BinOp, bool UseResidual = false, class BinIndexType = unsigned int, class BinCountType = unsigned int>
class BinUnaryOp2ArrayFunction : public BinUnaryFunctionBase<typename BinOp::result_type, BinIndexType, BinCountType>
{		
	typedef typename BinOp::second_argument_type op2_type;
	valarray<op2_type> op2_array;	// array of second argument passed to BinOp
	vector<short> is_bin_useful;	// vector<short> is faster than vector<bool>
	int numOfUsefulBins;

	void Init()
	{
		if (BinOp::IS_0_WHEN_OP2_IS_0 == true)
		{
			numOfUsefulBins = 0;
			for(unsigned int b = 0; b < op2_array.size(); b++)
			{
				is_bin_useful[b] = (op2_array[b] != 0);
				if (is_bin_useful[b]) numOfUsefulBins++;
			}
			//cout << numOfUsefulBins << " out of " << tar_hist.size() << " bins are useful" << endl;
		}
	}

protected:
	BinOp bin_op;

public:
	// as it is a constant, the condition on it should be removed by compiler
	enum { USE_RESIDUAL = UseResidual };

	static string Name() {	return BinOp::Name();	}

	// a stupid error occurs if using template for the input array
	//template<class VectorType>
	//BinUnaryOp2ArrayFunction(const VectorType& _op2_array, const BinOp& op = BinOp()) : bin_op(op)
	BinUnaryOp2ArrayFunction(const valarray<float>& _op2_array, const BinOp& op = BinOp()) : bin_op(op)
		,is_bin_useful(_op2_array.size(), true), op2_array(0.0f, _op2_array.size()), numOfUsefulBins(_op2_array.size())
	{
		for(unsigned int n = 0; n < _op2_array.size(); n++)	op2_array[n] = _op2_array[n];
		Init();
	}

	BinUnaryOp2ArrayFunction(const valarray<int>& _op2_array, const BinOp& op = BinOp()) : bin_op(op)
		,is_bin_useful(_op2_array.size(), true), op2_array(0.0f, _op2_array.size()), numOfUsefulBins(_op2_array.size())
	{
		for(unsigned int n = 0; n < _op2_array.size(); n++)	op2_array[n] = _op2_array[n];
		Init();
	}

	op2_type get_op2(const bin_index_type& b, const bin_count_type& c) const
	{
		if (USE_RESIDUAL)
		{
			assert(op2_array[b] >= c);
			return op2_array[b]-c;
		} 
		else return op2_array[b];
	}

	bin_result_type operator()(const bin_index_type& b, const bin_count_type& c) const	
	{	
		return bin_op(c,  get_op2(b, c));
	}

	bool IsBinUseful(const unsigned int& b) const	{	return static_cast<bool>(is_bin_useful[b]);	}

	// just for performance test, as the op2_array is not specified
	void SetHistLength(unsigned int hist_length)
	{
		op2_array.resize(hist_length);
		for(unsigned int n = 0; n < hist_length; n++)
			op2_array[n] = (rand() % 10000) / 10000.f;
	}
};

// normalize the first operand as histogram h is dynamically created
// usually needed when computing similarity with the second histogram known and normalized in prior
// UseResidual should always be false in this case?
template<class BinOp, bool UseResidual = false, class BinIndexType = unsigned int, class BinCountType = unsigned int>
class BinUnaryOp2ArrayNorm1Function : public BinUnaryOp2ArrayFunction<BinOp, UseResidual, BinIndexType, BinCountType>
{
	typedef BinUnaryOp2ArrayFunction<BinOp, UseResidual, BinIndexType, BinCountType> Base;

public:
	typedef typename BinOp::first_argument_type real_type;
	real_type Normalizer;

	template<class VectorType>
	BinUnaryOp2ArrayNorm1Function(const VectorType& _op2_array, const real_type& norm, const BinOp& op = BinOp()) : Base(_op2_array, op), Normalizer(norm)
	{
		// make sure the second histogram is already normalized
// 		float sum = 0.0f;
// 		for(unsigned int n = 0; n < _op2_array.size(); n++)	sum += _op2_array[n];
// 		assert(fabs(1.0f-sum) < 1e-6);
	}

	bin_result_type operator()(const bin_index_type& b, const bin_count_type& c) const
	{		
		return bin_op(c*Normalizer, get_op2(b, c));
	}
};

// normalize the second operand, used when the second histogram is not normalized
// UseResidual could be either true or false
template<class BinOp, bool UseResidual, class BinIndexType = unsigned int, class BinCountType = unsigned int>
class BinUnaryOp2ArrayNorm12Function : public BinUnaryOp2ArrayNorm1Function<BinOp, UseResidual, BinIndexType, BinCountType>
{
	typedef BinUnaryOp2ArrayNorm1Function<BinOp, UseResidual, BinIndexType, BinCountType> Base;

public:
	real_type Normalizer2;

	template<class VectorType>
	BinUnaryOp2ArrayNorm12Function(const VectorType& _op2_array, const real_type& norm, const real_type& norm2, const BinOp& op = BinOp()) 
		: Base(_op2_array, norm, op), Normalizer2(norm2)	{}

	bin_result_type operator()(const bin_index_type& b, const bin_count_type& c) const
	{
		return bin_op(c*Normalizer, get_op2(b, c)*Normalizer2);
	}
};