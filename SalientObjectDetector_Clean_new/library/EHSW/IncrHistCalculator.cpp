#include "IncrHistCalculator.h"
#include "IncrMultiHistCalculator.h"
#include "SlidingWindowCalculatorConcept.h"

unsigned int HistSparse::s_numOfAddCalls;
unsigned int HistSparse::s_numOfRemoveCalls;
unsigned int HistSparse::s_numOfInsert;
unsigned int HistSparse::s_numOfErase;

unsigned int HistDiffSparse::s_numOfAddCalls;
unsigned int HistDiffSparse::s_numOfRemoveCalls;
unsigned int HistDiffSparse::s_numOfInsert;
unsigned int HistDiffSparse::s_numOfErase;

//////////////////////////////////////////////////////////////////////////
//*
// concepts of histogram-based functions used in IncrHistCalculator
template<class ResultType, class PixelType>
class HistFuncDummy
{
public:
	typedef ResultType fun_result_type;
	unsigned int HistLength() const	{	return 0;	}

	void SetTotalCount(unsigned int TotalCount) {}

	template<class HistType>
	fun_result_type Evaluate(const HistType& hist) const	{	return 0;	}

	template<class HistType>
	fun_result_type Evaluate(const HistType& hist, unsigned int win_x, unsigned int win_y, unsigned int win_width, unsigned int win_height) const	
	{	return 0;	}

	template<class HistType>
	fun_result_type InitEvaluate(const HistType& hist)	{	return 0;	}

	void UpdateBin(const unsigned int& b, const unsigned int& count)	{}

	fun_result_type EvaluateAfterUpdate()	{	return 0;	}

	unsigned int numOfUpdateBinCalls;	// count of UpdateBinResult() calls, to compute histogram sparseness

	bool IsBinUseful(const PixelType& p) const	{	return true;	}
};

void incr_histogram_calculate_concept_checking()
{
	typedef InputImageDummy<unsigned int> InputImage;	InputImage img;
	typedef HistFuncDummy<float, unsigned int> HistFunc;	HistFunc func;

	IncrHistCalculationMethod calc_method;
	int range_X = 0, range_Y = 0, range_Width = 100, range_Height = 100, win_width = 10, win_height = 10;
	const bool column_wise = (win_height <= win_width);
	ResultWriterDummy<float> writer;
	incr_histogram_calculate(func, img, calc_method, range_X, range_Y, range_Width, range_Height, win_width, win_height, writer);

	{
		DEFINE_DenseSparseIncrCalculatorPV(InputImage, HistFunc)
		DenseSparseIncrCalculatorPV calculator(win_width, win_height, range_Width, range_Height, img, column_wise, func);
		incr_scanline_calculate(calculator, range_X, range_Y, writer);
	}

	{
		DEFINE_DenseSparseIncrCalculator(InputImage, HistFunc)
		DenseSparseIncrCalculator calculator(win_width, win_height, range_Width, range_Height, img, column_wise, func);
		incr_scanline_calculate(calculator, range_X, range_Y, writer);
	}

	{		
		typedef incr_histogram_calculator<scanline_sliding_window_calculator<InputImage, AccuHistBase<HistFunc, HistSparse>, HistSparse, false, true> > SparseSparseFullCalculator;
		SparseSparseFullCalculator calculator(win_width, win_height, range_Width, range_Height, img, column_wise, func);
		incr_scanline_calculate(calculator, range_Width, range_Height, writer);
	}
}
//*/

//////////////////////////////////////////////////////////////////////////
// usage of incr_histogram_calculate()
#include <HistUnarySimilarityFunction.h>
#include <HistBinarySimilarityFunction.h>

class SimpleBinImage
{
public:
	typedef unsigned int PixelType;
	vector<vector<PixelType> > data;
	SimpleBinImage(unsigned int w, unsigned int h) : data(h, vector<PixelType>(w, 0)) {}
	const PixelType* RowPtr(int row) const	{ return &data[row][0]; }
	const PixelType& Pixel(int x, int y) const {	return data[y][x];	}
	PixelType& Pixel(int x, int y) {	return data[y][x];	}

	unsigned int Height() const { return data.size(); }
	unsigned int Width() const { return data[0].size(); }
};

/*
void incr_histogram_calculate_usage()
{
	int hist_length = 1024;
	int image_width = 320, image_height = 240;
	SimpleBinImage bin_img(image_width, image_height);

	int range_X = 0, range_Y = 0, win_width = 60, win_height = 80;	

	OutputImageDummy<float> result_image(image_width, image_height);
	ImageResultWriter<OutputImageDummy<float>, OptimumFinderWriter<float, less<float>>> writer(result_image, win_width/2, win_width/2);

	// usage of HistUnarySimilarityFunction
	{
		valarray<float> target_hist(hist_length);	// create a normalized target histogram
		HistIntersectionFunc intersectino_func(target_hist, win_width*win_height);		
		incr_histogram_calculate(intersectino_func, bin_img, DenseSparseIncr, range_X, range_Y, image_width, image_height, win_width, win_height, writer);
		cout << writer.best_x << " " << writer.best_y << " " << writer.best_value << endl;
	}

	// usage of HistUnaryResidualSimilarityFunc
	{
		valarray<float> target_hist(0.0f, hist_length);	// create a un-normalized target histogram that contains the histogram of the sliding window
		// e.g., create it from the entire image
		for(int y = 0; y < image_height; y++)
			for(int x = 0; x < image_width; x++)
				target_hist[bin_img.Pixel(x, y)]++;

		HistIntersectionResidualFunc intersection_func(target_hist, win_width*win_height, image_width*image_height);
		incr_histogram_calculate(intersection_func, bin_img, DenseSparseIncr, range_X, range_Y, image_width, image_height, win_width, win_height, writer);
		cout << writer.best_x << " " << writer.best_y << " " << writer.best_value << endl;
	}

	// usage of HistBinarySimilarityFunction
	{
		int surr_win_width = win_width + 20;
		int surr_win_height = win_height + 20;
		HistCenterSurrIntersectionFunc func(hist_length, surr_win_width*surr_win_height, win_width*win_height);
		OutputImageDummy<float> result_image(image_width, image_height);
		ImageResultWriter<OutputImageDummy<float>, OptimumFinderWriter<float, less<float>>> writer(result_image, surr_win_width/2, surr_win_width/2);
		
		incr_2_histogram_calculate(func, bin_img, DenseSparseIncr, range_X, range_Y, image_width, image_height, surr_win_width, surr_win_height
			,range_X + (surr_win_width-win_width)/2, range_Y + (surr_win_height-win_height)/2, win_width, win_height, writer);
		cout << writer.best_x << " " << writer.best_y << " " << writer.best_value << endl;
	}
}
*/