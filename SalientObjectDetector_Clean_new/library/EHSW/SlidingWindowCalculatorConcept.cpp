// the algorithms are completely abstract, based on template and defined in head file
// this file is just for concept checking

#include "SlidingWindowCalculator.h"
#include "SlidingWindowCalculatorConcept.h"

int w, h;
int range_x, range_y, range_w, range_h;
const bool column_wise = (h <= w);

InputImageDummy<int> input;
ResultWriterDummy<float> result;
OutputImageDummy<float> output;

void test_incr_scanline_calculate_concept_checking()
{		
	{
		scanline_sliding_window_calculator<InputImageDummy<int>, AccuValueDummy<int>, ColValueDummy, false>
			calculator(w, h, range_w, range_h, input, column_wise);
		incr_scanline_calculate(calculator, range_x, range_y, result);
	}

	{
		scanline_sliding_window_calculator<InputImageDummy<int>, AccuValueDummy<int>, ColValueDummy, true>
			calculator(w, h, range_w, range_h, input, column_wise);
		incr_scanline_calculate(calculator, range_x, range_y, result);
	}

	{
		scanline_sliding_window_calculator_coldiff<InputImageDummy<int>, AccuValueDummy<int>, ColValueDummy, ColDiffValueDummy, false>
			calculator(w, h, range_w, range_h, input, column_wise);
		incr_scanline_calculate(calculator, range_x, range_y, result);
	}

	{
		scanline_sliding_window_calculator_coldiff<InputImageDummy<int>, AccuValueDummy<int>, ColValueDummy, ColDiffValueDummy, true>
			calculator(w, h, range_w, range_h, input, column_wise);
		incr_scanline_calculate(calculator, range_x, range_y, result);
	}

	{
		scanline_sliding_window_calculator<InputImageDummy<int>, AccuValueDummy<int>, ColValueDummy, false>
			calculator(w, h, range_w, range_h, input, column_wise);
		incr_scanline_calculate(calculator, range_x, range_y, ImageResultWriter<OutputImageDummy<float> >(output, 0, 0));
	}
	
	{
 		scanline_sliding_window_calculator<InputImageDummy<int>, SWSum<float>, SWSum<float>, false> 
 			calculator(w, h, range_w, range_h, input, column_wise);
 		incr_scanline_calculate(calculator, range_x, range_y, result);
	}

	{
		scanline_sliding_window_calculator_coldiff<InputImageDummy<int>, SWMean<float>, SWMean<float>, SWMean<float>, false>
			calculator(w, h, range_w, range_h, input, column_wise);
		incr_scanline_calculate(calculator, range_x, range_y, result);
	}	
}