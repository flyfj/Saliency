#pragma once

#include <HistBinaryFunction.h>

template<class RealType>
class BinReconstructionFunction : public BinFunctionBase<RealType>
{
private:
	valarray<int> global_histogram;		// of the entire image

public:
	BinReconstructionFunction(const valarray<int>& global_hist) 
		: global_histogram(global_hist) {}

	bin_result_type operator()(const bin_index_type& b, const bin_count_type& surround_count, const bin_count_type& center_count) const
	{
		// 1. equal to HistUnaryUniquenessFunc
		//assert(global_histogram[b] >= center_count);
		//bin_count_type bg_count = global_histogram[b] - center_count;
		//return (center_count <= bg_count) ? 0 : (center_count - bg_count);

		// 2. equal to HistCenterSurroundUniquenessFunc
		//assert(surround_count >= center_count);
		//bin_count_type ring_count = surround_count - center_count;
		//return (center_count <= ring_count) ? 0 : (center_count - ring_count);

		// 3. consider ring_count and bg_count simultaneously
		assert(surround_count >= center_count);
		bin_count_type ring_count = surround_count - center_count;

		if (center_count <= ring_count) return 0;	// if ring can fill, that's great
		bin_count_type remain_count = center_count - ring_count;

		assert(global_histogram[b] >= surround_count);
		float bg_count = (global_histogram[b] - surround_count) * 0.2;	// use pixels far from the center to fill but down weight them
		return (remain_count <= bg_count) ? 0 : (remain_count - bg_count);
	}
};

template<class RealType>
class HistBinaryCenterReconstructionFunction : public HistBinaryFunction<BinReconstructionFunction<RealType>, unary_multiply<RealType>>
{
	typedef HistBinaryFunction<BinReconstructionFunction<RealType>, unary_multiply<RealType>> Base;

public:
	HistBinaryCenterReconstructionFunction(const valarray<int>& global_hist, unsigned int CenterCount)
		: Base(global_hist.size(), BinReconstructionFunction<RealType>(global_hist), unary_multiply<RealType>(1.0f/(RealType)CenterCount)) {}
};