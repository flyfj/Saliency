#pragma once

#include <assert.h>
#include <numeric>
#include <valarray>
#include <fstream>
using namespace std;

#include "fast_histogram.h"
#include "SlidingWindowCalculator.h"

//template<class BinImage, class AccuHist, class ColHist>
//class incr_histogram_calculator : public scanline_sliding_window_calculator_col<BinImage, typename BinImage::PixelType, AccuHist, ColHist>

// BaseCalculator could be one of the following
// 1. scanline_sliding_window_calculator_col
// 2. scanline_sliding_window_calculator_col_pv
template<class BaseCalculator>
class incr_histogram_calculator : public BaseCalculator
{	
	typedef BaseCalculator BaseClass;
protected:
	typedef typename BaseClass::accu_value_type accu_hist_type;
	typedef typename BaseClass::col_value_type col_hist_type;

public:
	typedef typename BaseClass::input_image_type input_image_type;

	static string ConfigName()	{	return accu_hist_type::ConfigName() + "_" + col_hist_type::ConfigName() + "_" + BaseClass::ConfigName();	}
	typedef typename accu_hist_type::hist_function_type hist_function_type;

	incr_histogram_calculator(const int win_width, const int win_height, const int range_width, const int range_height
		, const input_image_type& img, bool column_wise, hist_function_type& hist_func)
		: BaseClass(win_width, win_height, range_width, range_height, img, column_wise)
	{
		// this is just for col and useless for coldiff
		new_row_value.resize(hist_func.HistLength());
		old_row_value.resize(hist_func.HistLength());
		
		SetFunction(hist_func);

		// 1. following line does not work for array_hash_map under debug but ok under release
		// col_values.clear();	col_values.resize(num_cols, col_hist_type(hist_func.HistLength()));

		// 2, col_values is already of correct size
		for(unsigned int n = 0; n < col_values.size(); n++)
			col_values[n].resize(hist_func.HistLength());
	}
};

// BaseCalculator could be one of the following
// 3. scanline_sliding_window_calculator_coldiff
// 4. scanline_sliding_window_calculator_coldiff_pv
template<class BaseCalculator>
class incr_histogram_calculator_coldiff : public incr_histogram_calculator<BaseCalculator>
{
	typedef incr_histogram_calculator<BaseCalculator> BaseClass;
public:
	incr_histogram_calculator_coldiff(const int win_width, const int win_height, const int range_width, const int range_height
		, const input_image_type& img, bool column_wise, hist_function_type& hist_func)
		: BaseClass(win_width, win_height, range_width, range_height, img, column_wise, hist_func)
	{
		// when baseclass is coldiff based, col_diff and row_diff needs resize()
		for(unsigned int n = 0; n < col_diff_values.size(); n++)
			col_diff_values[n].resize(hist_func.HistLength());
		row_diff.resize(hist_func.HistLength());
	}
};

//////////////////////////////////////////////////////////////////////////
// an implementation of incremental histogram calculator
//////////////////////////////////////////////////////////////////////////
// serve as the base class for accumulated_type for scanline_sliding_window_calculator_col()
template<class HistFuncType, class BaseHist>
class AccuHistBase : public BaseHist
{
	typedef BaseHist BaseClass;
protected:
	HistFuncType* hist_func;

public:
	typedef HistFuncType hist_function_type;
	typedef typename hist_function_type::fun_result_type fun_result_type;
	static string ConfigName() {	return hist_function_type::Name() + "_" + BaseHist::ConfigName();	}

	void dump() const	{}

	AccuHistBase(int) : hist_func(0) {}

	// set histogram computation function
	template<class HistFuncType>
	void SetFunction(HistFuncType& func)
	{
		hist_func = &func;
		resize(func.HistLength());
	}

	using BaseClass::operator+=;

	fun_result_type init_compute()	{	return hist_func->Evaluate(*this);	}	

	template<class HistType>
	fun_result_type update_and_compute(const HistType& add_hist, const HistType& remove_hist)
	{
		operator+=(add_hist);	operator-=(remove_hist);
		return hist_func->Evaluate(*this);
	}	

	bool IsUseful(const unsigned int& b) const	{	return hist_func->IsBinUseful(b);	}

	// directly compute a function that takes window as parameters
	fun_result_type init_compute(unsigned int win_x, unsigned int win_y, unsigned int win_width, unsigned int win_height)	
	{	
		return hist_func->Evaluate(*this, win_x, win_y, win_width, win_height);	
	}
	template<class HistType>
	fun_result_type update_and_compute(const HistType& add_hist, const HistType& remove_hist
		,unsigned int win_x, unsigned int win_y, unsigned int win_width, unsigned int win_height)
	{
		operator+=(add_hist);	operator-=(remove_hist);
		return hist_func->Evaluate(*this, win_x, win_y, win_width, win_height);
	}
};

template<class IncrHistFuncType>
class AccuHistSparseIncr : public AccuHistBase<IncrHistFuncType, HistSparse>
{
public:
	typedef AccuHistBase<IncrHistFuncType, HistSparse> Base;
	AccuHistSparseIncr(unsigned int COUNT) : Base(COUNT){}

	void dump() const	{}

	fun_result_type init_compute()	{	return hist_func->InitEvaluate(*this);	}	
	
	// H <= H + h1- h2, return histogram similarity
	fun_result_type update_and_compute(const HistSparse& add_hist, const HistSparse& remove_hist)
	{			
		for(const_iterator it = add_hist.begin(); it != add_hist.end(); it++)
		{
			const key_type& bin = it->first;
			iterator find_it = find(bin);
			if (find_it == end())	// if not there, add a new bin
				find_it = insert(make_pair(bin, 0)).first;

			hist_func->UpdateBin(bin, find_it->second += it->second);
		}

		for(const_iterator it = remove_hist.begin(); it != remove_hist.end(); it++)
		{
			const key_type& bin = it->first;
			const mapped_type& incr = it->second;
			
			iterator find_it = find(bin);	assert(find_it != end());
			if (find_it->second > incr)
			{
				hist_func->UpdateBin(bin, find_it->second -= incr);
			}
			else
			{
				if (it->second < incr) 
					cerr << "error in HistSparse::Remove() : bin " << bin << " is less than 0" << endl;
				erase(find_it);
				//hist_func->UpdateZeroBin(bin);
				hist_func->UpdateBin(bin, 0);
			}
		}
		
		return hist_func->EvaluateAfterUpdate();
	}

	// wrapper that takes window as parameters
	// this is just for compilation as it is impossible to incrementally compute a function dependent on window
	fun_result_type init_compute(unsigned int win_x, unsigned int win_y, unsigned int win_width, unsigned int win_height)
	{
		return init_compute();
	}

	fun_result_type update_and_compute(const HistSparse& add_hist, const HistSparse& remove_hist
		,unsigned int win_x, unsigned int win_y, unsigned int win_width, unsigned int win_height)
	{
		return update_and_compute(add_hist, remove_hist);
	}
};

#ifdef USING_ARRRY_HASH_MAP
typedef array_hash_map<int> hash_int_map_base;
#else
typedef hash_map<unsigned int, int> hash_int_map_base;
#endif

// an associate container that stores the difference of two sparse histogram
class HistDiffSparse : protected hash_int_map_base
{
	typedef hash_int_map_base BaseClass;
	unsigned int hist_length;

public:
	static unsigned int s_numOfAddCalls, s_numOfRemoveCalls;
	static unsigned int s_numOfInsert, s_numOfErase;

#ifdef USING_ARRRY_HASH_MAP
	static string ConfigName() {	return "ArrMap";	}
#else
	static string ConfigName() {	return "Map";	}
#endif	

	using BaseClass::const_iterator;
	using BaseClass::begin;
	using BaseClass::end;

	HistDiffSparse() : hist_length(0) {}
	//HistDiffSparse(const HistSparse& add, const HistSparse& remove)	{	create(add, remove);	}

	void create(const HistSparse& add, const HistSparse& remove)
	{
		assert(hist_length == add.Length());
		assert(add.Length() == remove.Length());

		for(HistSparse::const_iterator it = add.begin(); it != add.end(); it++)
			insert(make_pair(it->first, it->second));

		for(HistSparse::const_iterator it = remove.begin(); it != remove.end(); it++)
		{
			iterator find_it = find(it->first);
			if (find_it != end())
			{
				find_it->second -= it->second;
				if (find_it->second == 0) erase(find_it);
			}
			else insert(make_pair(it->first, -static_cast<int>(it->second)));
		}
	}

	void resize(unsigned int length) 
	{ 
		hist_length = length;
#ifdef USING_ARRRY_HASH_MAP	
		BaseClass::resize(length);
#endif
	}

	void reset()	{	clear();	}

	void operator+=(const unsigned int& b) 
	{ 
		s_numOfAddCalls++;
		iterator it = find(b);
		if (it != end())
		{
			it->second++;
			if (it->second == 0) 
			{
				erase(it);
				s_numOfErase++;
			}
		}
		else 
		{
			insert(make_pair(b, 1));
			s_numOfInsert++;
		}
	}

	void operator-=(const unsigned int& b)
	{
		s_numOfRemoveCalls++;
		iterator it = find(b);
		if (it != end())
		{
			it->second--;
			if (it->second == 0) 
			{
				erase(it);
				s_numOfErase++;
			}
		}
		else 
		{
			insert(make_pair(b, -1));
			s_numOfInsert++;
		}
	}
};

template<class IncrHistFuncType>
class AccuHistArrayIncr : public AccuHistBase<IncrHistFuncType, HistArray>
{
public:
	typedef AccuHistBase<IncrHistFuncType, HistArray> Base;
	AccuHistArrayIncr(unsigned int COUNT) : Base(COUNT) {}

	fun_result_type init_compute()
	{
		return hist_func->InitEvaluate(*this);
	}

	fun_result_type add_and_compute(const HistDiffSparse& diff_hist)
	{
		for(HistDiffSparse::const_iterator it = diff_hist.begin(); it != diff_hist.end(); it++)
			hist_func->UpdateBin(it->first, (*this)[it->first] += it->second);
		return hist_func->EvaluateAfterUpdate();
	}

	fun_result_type remove_and_compute(const HistDiffSparse& diff_hist)
	{
		for(HistDiffSparse::const_iterator it = diff_hist.begin(); it != diff_hist.end(); it++)
			hist_func->UpdateBin(it->first, (*this)[it->first] -= it->second);
		return hist_func->EvaluateAfterUpdate();
	}

	// H <= H + h1- h2, return histogram similarity
	//*
	fun_result_type update_and_compute(const HistSparse& add_hist, const HistSparse& remove_hist)
	{
		//num_update_and_compute++;
		//total_ops.push_back(add_hist.size() + remove_hist.size());	// this is very slow!
		
		// 1. combine the two sparse histograms, this is very slow
// 		HistDiffSparse diff_hist(add_hist, remove_hist);
// 		for(HistDiffSparse::const_iterator it = diff_hist.begin(); it != diff_hist.end(); it++)
// 			hist_func->UpdateBin(it->first, (*this)[it->first] += it->second);
		// diff_ops.push_back(diff_hist.size()); // this is very slow
		
		// 2. traverse the two sparse histogram separately
		// computation on bins at which the two histograms cancel out is wasted
		// also if add_hist is before remove_hist, the bin value could be out of max range 
		// and cause problem for certain functions that assume this condition, e.g., HistFastEntropyFunc
		for(HistSparse::const_iterator it = remove_hist.begin(); it != remove_hist.end(); it++)
			hist_func->UpdateBin(it->first, (*this)[it->first] -= it->second);

		for(HistSparse::const_iterator it = add_hist.begin(); it != add_hist.end(); it++)
			hist_func->UpdateBin(it->first, (*this)[it->first] += it->second);
		
		return hist_func->EvaluateAfterUpdate();
	}
	//*/

	// 3. avoid the unnecessary computation in 2 and ensure the bin value is within [0, max value]
	/*
	fun_result_type update_and_compute(const HistSparse& cadd_hist, const HistSparse& cremove_hist)	
	{		
		HistSparse& add_hist = const_cast<HistSparse&>(cadd_hist);
		HistSparse& remove_hist = const_cast<HistSparse&>(cremove_hist);
		if (add_hist.Length() < remove_hist.Length())
		{
			for(HistSparse::const_iterator it = add_hist.begin(); it != add_hist.end(); it++)
			{
				value_type remove_count;
				if (remove_hist.find_and_mark(it->first, remove_count))
				{
					int incr = it->second - remove_count;	// cannot use value_type for incr, as it may be unsigned
					if (incr != 0)	hist_func->UpdateBin(it->first, (*this)[it->first] += incr);
				}
				else hist_func->UpdateBin(it->first, (*this)[it->first] += it->second);
			}

			for(HistSparse::iterator it = remove_hist.begin(); it != remove_hist.end(); it++)
			{
				if (it->second < 0)	it->second = -it->second;	// already visited, restore and skip it
				else hist_func->UpdateBin(it->first, (*this)[it->first] -= it->second);
			}
		}
		else
		{
			for(HistSparse::const_iterator it = remove_hist.begin(); it != remove_hist.end(); it++)
			{
				value_type add_count;
				if (add_hist.find_and_mark(it->first, add_count))
				{
					int incr = add_count - it->second;	// cannot use value_type for incr, as it may be unsigned
					if (incr != 0) hist_func->UpdateBin(it->first, (*this)[it->first] += incr);
				}
				else hist_func->UpdateBin(it->first, (*this)[it->first] -= it->second);
			}

			for(HistSparse::iterator it = add_hist.begin(); it != add_hist.end(); it++)
			{
				if (it->second < 0)	it->second = -it->second;	// already visited, restore and skip it
				else hist_func->UpdateBin(it->first, (*this)[it->first] += it->second);
			}
		}
		return hist_func->EvaluateAfterUpdate();
	}
	//*/	

	fun_result_type update_and_compute(const HistArray& add_hist, const HistArray& remove_hist)
	{
		//num_update_and_compute++;
		/*
		// 1. traverse the two sparse histogram separately
		// computation on bins at which the two histograms cancel out is wasted
		// also if add_hist is before remove_hist, the bin value could be out of max range 
		// and cause problem for certain functions that assume this condition, e.g., HistFastEntropyFunc
		for(unsigned int n = 0; n < remove_hist.size(); n++)
		if (remove_hist[n] != 0)
		hist_func->UpdateBin(n, (*this)[n] -= remove_hist[n]);

		for(unsigned int n = 0; n < add_hist.size(); n++)
			if (add_hist[n] != 0)
				hist_func->UpdateBin(n, (*this)[n] += add_hist[n]);
		*/

		// 2. avoid the unnecessary computation in 2 and ensure the bin value is within [0, max value]
		for(unsigned int n = 0; n < remove_hist.size(); n++)
		{
			int incr = add_hist[n] - remove_hist[n];	// cannot use value_type for incr, as it may be unsigned
			if (incr != 0)
				hist_func->UpdateBin(n, (*this)[n] += incr);
		}

		return hist_func->EvaluateAfterUpdate();
	}

	// wrapper that takes window as parameters
	// this is just for compilation as it is impossible to incrementally compute a function dependent on window
	fun_result_type init_compute(unsigned int win_x, unsigned int win_y, unsigned int win_width, unsigned int win_height)
	{
		return init_compute();
	}

	template<class HistType>
	fun_result_type update_and_compute(const HistType& add_hist, const HistType& remove_hist
		,unsigned int win_x, unsigned int win_y, unsigned int win_width, unsigned int win_height)
	{
		return update_and_compute(add_hist, remove_hist);
	}
};

//////////////////////////////////////////////////////////////////////////
// each configuration specifies data structure of histogram and partial histogram
// and histogram function evaluation method
// ideally there should be 8 configurations
// the 2 configurations (sparse, dense, *) makes no sense so there are 6 valid configurations

// 1. dense +/- dense, compute all bin, the standard distributive approach
#define DEFINE_DenseDenseFullCalculator(BinImage, HistFuncType)\
	typedef incr_histogram_calculator<scanline_sliding_window_calculator<BinImage, AccuHistBase<HistFuncType, HistArray>, HistArray, false> > DenseDenseFullCalculator;

// 2. dense +/- sparse, compute all bin, modified from 1 via using sparse representation for partial histogram
#define DEFINE_DenseSparseFullCalculator(BinImage, HistFuncType)\
	typedef incr_histogram_calculator<scanline_sliding_window_calculator<BinImage, AccuHistBase<HistFuncType, HistArray>, HistSparse, false> > DenseSparseFullCalculator;

// 2.2 H += h1-h2, not implemented currently
//#define DEFINE_DenseSparsePlusFullCalculator(BinImage, HistFuncType)\
//	typedef incr_histogram_calculator_coldiff<BinImage, AccuHistBase<HistFuncType, HistArray>, HistSparse, HistDiffSparse> DenseSparsePlusFullCalculator;

// 3. sparse +/- sparse, compute all bin, modified from 2 via also using sparse representation for histogram
#define DEFINE_SparseSparseFullCalculator(BinImage, HistFuncType)\
	typedef incr_histogram_calculator<scanline_sliding_window_calculator<BinImage, AccuHistBase<HistFuncType, HistSparse>, HistSparse, false> > SparseSparseFullCalculator;

// 4. dense +/- sparse, compute changed bin, modified from 2 via using incremental computation of histogram function
// 4.1, H += h1, H -= h2
#define DEFINE_DenseSparseIncrCalculator(BinImage, HistFuncType)\
	typedef incr_histogram_calculator<scanline_sliding_window_calculator<BinImage, AccuHistArrayIncr<HistFuncType>, HistSparse, false> > DenseSparseIncrCalculator;

#define DEFINE_DenseSparseIncrCalculatorPV(BinImage, HistFuncType)\
	typedef incr_histogram_calculator<scanline_sliding_window_calculator<BinImage, AccuHistArrayIncr<HistFuncType>, HistSparse, true> > DenseSparseIncrCalculatorPV;

// 4.2, H += h1-h2, modified from 4.1 via using further sparse representation for partial histogram
// this save time in H computation but increase time for h1-h2 maintenance, better for complex histogram function
// should it also be better for smooth image since change in h1-h2 should be very small?
// for simple bin op such as intersection, 4.1 is faster than 4.2
#define DEFINE_DenseSparsePlusIncrCalculator(BinImage, HistFuncType)\
	typedef incr_histogram_calculator_coldiff<scanline_sliding_window_calculator_coldiff<BinImage, AccuHistArrayIncr<HistFuncType>, HistSparse, HistDiffSparse, false> > DenseSparsePlusIncrCalculator;

#define DEFINE_DenseSparsePlusIncrCalculatorPV(BinImage, HistFuncType)\
	typedef incr_histogram_calculator_coldiff<scanline_sliding_window_calculator_coldiff<BinImage, AccuHistArrayIncr<HistFuncType>, HistSparse, HistDiffSparse, true> > DenseSparsePlusIncrCalculatorPV;

// 5. sparse +/- sparse, compute changed bin, modified from 4 via using sparse representation for histogram
// this configuration makes no sense since computation time is O(partial histogram) and using sparse histogram is not useful
#define DEFINE_SparseSparseIncrCalculator(BinImage, HistFuncType)\
	typedef incr_histogram_calculator<scanline_sliding_window_calculator<BinImage, AccuHistSparseIncr<HistFuncType>, HistSparse, false> > SparseSparseIncrCalculator;

// 6. dense +/- dense, compute changed bin
#define DEFINE_DenseDenseIncrCalculator(BinImage, HistFuncType)\
	typedef incr_histogram_calculator<scanline_sliding_window_calculator<BinImage, AccuHistArrayIncr<HistFuncType>, HistArray, false> > DenseDenseIncrCalculator;

enum IncrHistCalculationMethod
{ 
	DenseDenseFull		=	1
	,DenseSparseFull
	,SparseSparseFull

	,DenseSparseIncr
	,DenseSparseIncrPV
	,DenseSparsePlusIncr
	,DenseSparsePlusIncrPV

	,SparseSparseIncr

	,DenseDenseIncr
};

inline string IncrHistCalculationMethod2Name(int method)
{
	switch (method)
	{
	case DenseDenseFull:	return "DenseDenseFull";
	case DenseSparseFull:	return "DenseSparseFull";
	case SparseSparseFull:	return "SparseSparseFull";
	case DenseSparseIncr:	return "DenseSparseIncr";
	case DenseSparseIncrPV:	return "DenseSparseIncrPV";
	case DenseSparsePlusIncr:	return "DenseSparsePlusIncr";
	case DenseSparsePlusIncrPV:	return "DenseSparsePlusIncrPV";
	case SparseSparseIncr:	return "SparseSparseIncr";
	case DenseDenseIncr:	return "DenseDenseIncr";
	default:	return "UnknownMethod";
	}
}

// a wrapper method
template<class HistFuncType, class BinImage, class ResultWriter>
pair<float, float> incr_histogram_calculate(HistFuncType& func, const BinImage& img, int method, const int range_X, const int range_Y, const int range_width, const int range_height, const int win_width, const int win_height, ResultWriter& writer)
{
	int num_scanned_pixels;

	HistSparse::s_numOfAddCalls = 0;	HistSparse::s_numOfRemoveCalls = 0;
	HistSparse::s_numOfInsert = 0;		HistSparse::s_numOfErase = 0;

	const bool column_wise = (win_height <= win_width);

	switch (method)
	{		
	case DenseDenseFull:
		{
			DEFINE_DenseDenseFullCalculator(BinImage, HistFuncType)
			DenseDenseFullCalculator calculator(win_width, win_height, range_width, range_height, img, column_wise, func);
			num_scanned_pixels = incr_scanline_calculate(calculator, range_X, range_Y, writer);
		}
		break;
		
	case DenseSparseFull:
		{
			DEFINE_DenseSparseFullCalculator(BinImage, HistFuncType)
			DenseSparseFullCalculator calculator(win_width, win_height, range_width, range_height, img, column_wise, func);
			num_scanned_pixels = incr_scanline_calculate(calculator, range_X, range_Y, writer);
		}
		break;

	case SparseSparseFull:
		{
			DEFINE_SparseSparseFullCalculator(BinImage, HistFuncType)
				SparseSparseFullCalculator calculator(win_width, win_height, range_width, range_height, img, column_wise, func);
			num_scanned_pixels = incr_scanline_calculate(calculator, range_X, range_Y, writer);
		}
		break;
		
	case DenseSparseIncr:
		{
			DEFINE_DenseSparseIncrCalculator(BinImage, HistFuncType)
			DenseSparseIncrCalculator calculator(win_width, win_height, range_width, range_height, img, column_wise, func);
			num_scanned_pixels = incr_scanline_calculate(calculator, range_X, range_Y, writer);
		}
		break;

	case DenseSparseIncrPV:
		{
			DEFINE_DenseSparseIncrCalculatorPV(BinImage, HistFuncType)
				DenseSparseIncrCalculatorPV calculator(win_width, win_height, range_width, range_height, img, column_wise, func);
			num_scanned_pixels = incr_scanline_calculate(calculator, range_X, range_Y, writer);
		}
		break;

	case DenseSparsePlusIncr:
		{
			DEFINE_DenseSparsePlusIncrCalculator(BinImage, HistFuncType)
				DenseSparsePlusIncrCalculator calculator(win_width, win_height, range_width, range_height, img, column_wise, func);
			num_scanned_pixels = incr_scanline_calculate(calculator, range_X, range_Y, writer);
		}
		break;

	case DenseSparsePlusIncrPV:
		{
			DEFINE_DenseSparsePlusIncrCalculatorPV(BinImage, HistFuncType)
				DenseSparsePlusIncrCalculatorPV calculator(win_width, win_height, range_width, range_height, img, column_wise, func);
			num_scanned_pixels = incr_scanline_calculate(calculator, range_X, range_Y, writer);
		}
		break;

	case SparseSparseIncr:
		{
			DEFINE_SparseSparseIncrCalculator(BinImage, HistFuncType)
				SparseSparseIncrCalculator calculator(win_width, win_height, range_width, range_height, img, column_wise, func);
			num_scanned_pixels = incr_scanline_calculate(calculator, range_X, range_Y, writer);
		}
		break;

	case DenseDenseIncr:
		{
			DEFINE_DenseDenseIncrCalculator(BinImage, HistFuncType)
				DenseDenseIncrCalculator calculator(win_width, win_height, range_width, range_height, img, column_wise, func);
			num_scanned_pixels = incr_scanline_calculate(calculator, range_X, range_Y, writer);
		}
		break;

	default:	cerr << "unknown calculator method " << method << endl;	break;
	}

	int numTotalCalls = HistSparse::s_numOfAddCalls + HistSparse::s_numOfRemoveCalls;

	float memory_operation_ratio = (numTotalCalls == 0) ? 0.0f : ((HistSparse::s_numOfInsert+HistSparse::s_numOfErase) / (float)numTotalCalls);

	return make_pair(func.numOfUpdateBinCalls / (float)(num_scanned_pixels), memory_operation_ratio);
}

#include <map>

// condense a distribution (sum=1) by removing entries with very small values (accumulation < percent)
// return (#non-zero entries before, #removed entries)
template<class VectorType>
pair<unsigned int, unsigned int> condense_distribution(VectorType& dist, float percent)
{
	assert(percent > 0);
	assert(percent < 1);
	multimap<float, int> sorted_entries;
	unsigned int LENGTH = dist.size();

	for(unsigned int n = 0; n < LENGTH; n++)
		if (dist[n] > 0) sorted_entries.insert(make_pair(dist[n], n));

	unsigned int num_removed = 0;
	float sum = 0.0f;	// accumulated removed values
	multimap<float, int>::iterator it;
	for(it = sorted_entries.begin(); it != sorted_entries.end(); it++)
	{
		sum += it->first;
		if (sum <= percent)	// remove the entry
		{
			dist[it->second] = 0;
			num_removed++;
		}
		else 
		{
			sum -= it->first;
			break;
		}
	}
	// normalize
	float normalizer = 1/(1-sum);
	for(unsigned int n = 0; n < dist.size(); n++)
		dist[n] *= normalizer;

	return make_pair(sorted_entries.size(), num_removed);
}