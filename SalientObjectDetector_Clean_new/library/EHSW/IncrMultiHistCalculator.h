#pragma once

#include "IncrHistCalculator.h"

template<int OperandNumber, class HistBinaryFuncType, class BaseHistType>
typename HistBinaryFuncType::fun_result_type update_hist_operand_and_compute(HistBinaryFuncType& hist_func
		,const HistSparse& add_hist, const HistSparse& remove_hist, BaseHistType& hist1, BaseHistType& hist2)
{
	// 2. traverse the two sparse histogram separately, computation will be wasted on those bins 
	// at which the values from the two histograms are equal
	for(HistSparse::const_iterator it = add_hist.begin(); it != add_hist.end(); it++)
		hist_func.UpdateBin<OperandNumber>(it->first, it->second, hist1, hist2);

	for(HistSparse::const_iterator it = remove_hist.begin(); it != remove_hist.end(); it++)
		hist_func.UpdateBin<OperandNumber>(it->first, -it->second, hist1, hist2);

	return hist_func.EvaluateAfterUpdate();
}

template<class HistBinaryFuncType, class Calculator1, class Calculator2, class ResultWriter>
int incr_2_scanline_calculate(HistBinaryFuncType& func, Calculator1& calc1, const int range_X1, const int range_Y1
							  ,Calculator2& calc2, const int range_X2, const int range_Y2
							  ,ResultWriter& result)
{
	// at left-top corner, init first w columns
	int x1 = range_X1, x2 = range_X2;
	unsigned int win_pos_y1 = range_Y1, win_pos_y2 = range_Y2;

	int dx = 1;	// increment of x coordinate, 1 or -1

	int col1 = 0, col2 = 0;
	// initialize first w column values and compute result of initial window
	for(; col1 < static_cast<int>(calc1.win_width); col1++, x1++)	
		calc1.init_column_and_add(col1, x1, win_pos_y1);

	for(; col2 < static_cast<int>(calc2.win_width); col2++, x2++)
		calc2.init_column_and_add(col2, x2, win_pos_y2);

	result.init_write(static_cast<typename ResultWriter::value_type>(func.InitEvaluate(calc1, calc2)));

	// initialize remaining column histograms and compute remaining values in first row
	for(; col1 < static_cast<int>(calc1.range_width); col1++, x1++, col2++, x2++)
	{		
		calc1.init_column(col1, x1, win_pos_y1);
		update_hist_operand_and_compute<1>(func, calc1.col_values[col1], calc1.skip_column(col1, 1), calc1, calc2);
		calc2.init_column(col2, x2, win_pos_y2);
		result.incrx_write(dx, static_cast<typename ResultWriter::value_type>(
			update_hist_operand_and_compute<2>(func, calc2.col_values[col2], calc2.skip_column(col2, 1), calc1, calc2)));
	}
	
	// row-by-row incremental calculation
	while (win_pos_y1+calc1.win_height < range_Y1+calc1.range_height)
	{
		dx = -dx;		// invert the scanning direction
		win_pos_y1++;	win_pos_y2++;	// go to next row

		col1 = (dx > 0) ? 0 : calc1.range_width-1;
		col2 = (dx > 0) ? 0 : calc2.range_width-1;

		const Calculator1::pixel_type* new_row_ptr1 = calc1.input_image.RowPtr(win_pos_y1 + calc1.win_height-1);	new_row_ptr1 += range_X1+col1;
		const Calculator1::pixel_type* old_row_ptr1 = calc1.input_image.RowPtr(win_pos_y1-1);	old_row_ptr1 += range_X1+col1;

		const Calculator2::pixel_type* new_row_ptr2 = calc2.input_image.RowPtr(win_pos_y2 + calc2.win_height-1);	new_row_ptr2 += range_X2+col2;
		const Calculator2::pixel_type* old_row_ptr2 = calc2.input_image.RowPtr(win_pos_y2-1);	old_row_ptr2 += range_X2+col2;

		// update current w column histograms
		calc1.before_go_next_row();	calc2.before_go_next_row();
		unsigned int counter = 0;

		for(; counter < calc2.win_width; col2+=dx, counter++, new_row_ptr2+=dx, old_row_ptr2+=dx)
			calc2.update_column_and_add(col2, *new_row_ptr2, *old_row_ptr2);
		
		for(counter = 0; counter < calc1.win_width; col1+=dx, counter++, new_row_ptr1+=dx, old_row_ptr1+=dx)
			calc1.update_column_and_add(col1, *new_row_ptr1, *old_row_ptr1);

		update_hist_operand_and_compute<1>(func, calc1.new_row_value, calc1.old_row_value, calc1, calc2);
		result.incry_write(1, static_cast<typename ResultWriter::value_type>(
			update_hist_operand_and_compute<2>(func, calc2.new_row_value, calc2.old_row_value, calc1, calc2)));
		
		// sweep remaining columns
 		for(; counter < calc1.range_width; col1+=dx, col2+=dx, counter++, new_row_ptr1+=dx, old_row_ptr1+=dx, new_row_ptr2+=dx, old_row_ptr2+=dx)
		{
			update_hist_operand_and_compute<1>(func, calc1.update_column(col1, *new_row_ptr1, *old_row_ptr1), calc1.skip_column(col1, dx), calc1, calc2);
 			result.incrx_write(dx, static_cast<typename ResultWriter::value_type>(
				update_hist_operand_and_compute<2>(func, calc2.update_column(col2, *new_row_ptr2, *old_row_ptr2), calc2.skip_column(col2, dx), calc1, calc2)));
		}
	}

	return 0;
}

// 4. dense +/- sparse, compute changed bin, modified from 2 via using incremental computation of histogram function
// 4.1, H += h1, H -= h2
// #define DEFINE_DenseSparseIncrCalculator2(BinImage, HistBinaryFuncType)\
// 	typedef incr_histogram_calculator<scanline_sliding_window_calculator_col<BinImage, AccuHistArrayIncr<HistBinaryFuncType>, HistSparse> > DenseSparseIncrCalculator2;

template<class HistBinaryFuncType, class BinImage, class ResultWriter>
pair<float, float> incr_2_histogram_calculate(HistBinaryFuncType& func, const BinImage& img, int method
		,const int range_X1, const int range_Y1, const int range_width, const int range_height, const int win_width1, const int win_height1
		,const int range_X2, const int range_Y2, const int win_width2, const int win_height2
		,ResultWriter& writer)
{
	//const bool column_wise = (win_height1 <= win_width1);
	const bool column_wise = true;

	switch (method)
	{
		case DenseSparseIncr:
			{
				DEFINE_DenseSparseIncrCalculator(BinImage, HistBinaryFuncType)
					DenseSparseIncrCalculator calculator1(win_width1, win_height1, range_width, range_height, img, column_wise, func);

				DEFINE_DenseSparseIncrCalculator(BinImage, HistBinaryFuncType)
					DenseSparseIncrCalculator calculator2(win_width2, win_height2, range_width+win_width2-win_width1, range_height+win_height2-win_height1, img, column_wise, func);
				incr_2_scanline_calculate(func, calculator1, range_X1, range_Y1, calculator2, range_X2, range_Y2, writer);
			}
			break;

		default:	cerr << "unsupported calculator method : " << IncrHistCalculationMethod2Name(method) << endl;	break;
	}

	return make_pair(0.0f, 0.0f);
}