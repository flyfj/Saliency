#include "IncrMultiHistCalculator.h"
#include "SlidingWindowCalculatorConcept.h"

#include "HistBinaryFunction.h"
#include "HistBinarySimilarityFunction.h"

template<class ResultType>
class HistBinaryFuncDummy
{
public:
	typedef ResultType fun_result_type;
	unsigned int HistLength() const	{	return 0;	}

	//void SetTotalCount(unsigned int TotalCount) {}

	template<class Hist1Type, class Hist2Type>
	fun_result_type InitEvaluate(const Hist1Type& hist1, const Hist2Type& hist2)	{	return 0;	}

	template<int OperandNumber, class HistType>
	void UpdateBin(const unsigned int& b, const int& delta, HistType& hist1, HistType& hist2) {}

	fun_result_type EvaluateAfterUpdate()	{	return 0;	}

	unsigned int numOfUpdateBinCalls;	// count of UpdateBinResult() calls, to compute histogram sparseness
};

typedef InputImageDummy<unsigned int> InputImage;
//typedef HistBinaryFuncDummy<float> HistBinaryFunc;	HistBinaryFunc func;

void incr_2_histogram_calculate_concept_checking()
{
	InputImage img;	
	IncrHistCalculationMethod calc_method;
	int range_X1 = 0, range_Y1 = 0, range_Width = 100, range_Height = 100, win_width1 = 30, win_height1 = 30;
	int range_X2 = 10, range_Y2 = 10, win_width2 = 10, win_height2 = 10;

	ResultWriterDummy<float> writer;

	HistCenterSurrIntersectionFunc func(10, 10, 0);
	
	incr_2_histogram_calculate(func, img, calc_method, range_X1, range_Y1, range_Width, range_Height, win_width1, win_height1
		,range_X2, range_Y2, win_width2, win_height2, writer);
}