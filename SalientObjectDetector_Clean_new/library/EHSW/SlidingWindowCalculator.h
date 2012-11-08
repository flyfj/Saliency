#pragma once

#include <string>
#include <vector>
using namespace std;

#include "SlidingWindowResultWriter.h"

// sum of sliding window 
template <class _Ty>
class SWSum
{
public:	
	typedef _Ty fun_result_type;
	typedef SWSum<fun_result_type> MyClass;

	SWSum() : sum(0){}
	explicit SWSum(unsigned int COUNT) : sum(0){}
	void reset()	{	sum = 0;	}
		
	void operator+=(const fun_result_type& v)	{	sum += v;	}	
	void operator-=(const fun_result_type& v)	{	sum -= v;	}
	void operator+=(const MyClass& i)	{	sum += i.sum;	}
	
	fun_result_type init_compute()	{	return sum;	}
	fun_result_type init_compute(unsigned int win_x, unsigned int win_y, unsigned int win_width, unsigned int win_height)
	{
		return init_compute();
	}
	
	fun_result_type update_and_compute(const SWSum& add, const SWSum& remove)	{	return sum += add.sum-remove.sum;	}
	fun_result_type update_and_compute(const SWSum& add, const SWSum& remove, unsigned int win_x, unsigned int win_y, unsigned int win_width, unsigned int win_height)
	{
		return update_and_compute(add, remove);
	}
	
	// this is just for testing scanline_sliding_window_calculator_coldiff
	// which is slower than scanline_sliding_window_calculator_col for sum(mean) computation
	void create(const SWSum& add, const SWSum& remove)	{	sum = add.sum-remove.sum;	}
	fun_result_type add_and_compute(const SWSum& diff)		{	return sum += diff.sum;	}
	fun_result_type remove_and_compute(const SWSum& diff)	{	return sum -= diff.sum;	}

	// just for compilation
	bool IsUseful(const fun_result_type& v) { return true; }

protected:
	fun_result_type sum;
};

// mean of sliding window 
template <class _Ty>
class SWMean : public SWSum<_Ty>
{
public:	
	SWMean() : normalizer(1.0f) {}
	explicit SWMean(unsigned int PixelCount) : normalizer(1.0f/PixelCount) {}		

	fun_result_type init_compute()	{	return static_cast<fun_result_type>(sum * normalizer);	}
	fun_result_type init_compute(unsigned int win_x, unsigned int win_y, unsigned int win_width, unsigned int win_height)
	{
		return init_compute();
	}
	fun_result_type update_and_compute(const SWMean& add, const SWMean& remove)	{	return static_cast<fun_result_type>((sum += add.sum-remove.sum) * normalizer);	}
	fun_result_type update_and_compute(const SWMean& add, const SWMean& remove, unsigned int win_x, unsigned int win_y, unsigned int win_width, unsigned int win_height)
	{
		return update_and_compute(add, remove);
	}

	fun_result_type add_and_compute(const SWMean& diff)		{	return (sum += diff.sum) * normalizer;	}
	fun_result_type remove_and_compute(const SWMean& diff)	{	return (sum -= diff.sum) * normalizer;	}

	// just for compilation
	bool IsUseful(const fun_result_type& v) { return true; }

private:
	float normalizer;
};

// if ValidatePixel = true, for every input pixel, test whether it is useful for accu_value computation
// if PassWindowPara = true, pass the window (x, y, w, h) as parameters to the computation
template<class InputImage, class AccuValue, class ColValue, bool ValidatePixel, bool PassWindowPara = false>
//class scanline_sliding_window_calculator : protected AccuValue	// using protected is ok for sliding window computation
class scanline_sliding_window_calculator : public AccuValue			// using public is for multi sliding window as AccuValue needs to be accessed outside
{
protected:
	typedef AccuValue accu_value_type;	// accumulated value of the search window
	typedef typename accu_value_type::fun_result_type fun_result_type;
	typedef ColValue col_value_type;	// accumulated values for all columns(rows)
	
	unsigned int WinSkip() const { return is_column_wise ? win_width : win_height; }

public:	
	// as it is a constant, the condition on it should be removed by compiler
	enum { DO_PIXEL_VALIDATION = ValidatePixel };
	enum { PASS_WINDOW_AS_PARAMETER = PassWindowPara };

	static string ConfigName()	{	return DO_PIXEL_VALIDATION ? "ColPV" : "Col";	}

	typedef InputImage input_image_type;
	typedef typename input_image_type::PixelType pixel_type;

	//accu_value_type accu_value;
	vector<col_value_type> col_values;				// using public is for multi sliding window computation
	col_value_type new_row_value, old_row_value;	// using public is for multi sliding window computation

	const InputImage& input_image;
	const bool is_column_wise;	// if false, switch the role of row and column

	unsigned int win_pos_x, win_pos_y;			// window left-top, used in scanning (incr_scanline_calculate), could be useful for certain functions
	const unsigned int win_width, win_height;	// window dimension
	const unsigned int range_width, range_height;	// search range dimension, must be a member of calculator to initialize the column values	

	scanline_sliding_window_calculator(const unsigned int width, const unsigned int height
		,const unsigned int rwidth, const unsigned int rheight, const InputImage& img, bool column_wise)
		: AccuValue(win_width * win_height), win_width(width), win_height(height), range_width(rwidth), range_height(rheight), input_image(img)
		// 1. obsolete, when only using column histograms
		//,is_column_wise(true), col_values(range_width)
		// 2. determine whether to use column or row histograms by their relative order
		//,is_column_wise(win_height <= win_width), col_values((win_height <= win_width) ? rwidth : rheight)
		// 3. determined by the caller
		,is_column_wise(column_wise), col_values(column_wise ? rwidth : rheight)
	{}

	fun_result_type init_compute()
	{
		return PASS_WINDOW_AS_PARAMETER ? AccuValue::init_compute(win_pos_x, win_pos_y, win_width, win_height) : AccuValue::init_compute();
	}

	// return true if the column value is changed
	bool init_column(int col, int x, int y)			// using public for multi sliding window computation
	{
		ColValue& col_value = col_values[col];
		col_value.reset();

		bool changed = DO_PIXEL_VALIDATION ? false : true;

		if (is_column_wise)
			for(unsigned int row = 0; row < win_height; row++, y++)
			{
				const pixel_type& p = input_image.Pixel(x, y);
				if (DO_PIXEL_VALIDATION)
				{
					if (IsUseful(p))
					{
						col_value += p;
						changed = true;
					}
				} 
				else
				{
					col_value += p;
				}				
			}
		else			
			for(unsigned int row = 0; row < win_width; row++, x++)	// here row is actually column
			{
				const pixel_type& p = input_image.Pixel(x, y);
				if (DO_PIXEL_VALIDATION)
				{
					if (IsUseful(p))
					{
						col_value += p;
						changed = true;
					}
				} 
				else
				{
					col_value += p;
				}				
			}

		return changed;
	}
	
	void init_column_and_add(int col, int x, int y)
	{
		if (init_column(col, x, y))
			(*this) += (col_values[col]);
	}

	// compute remain values in first row
	fun_result_type init_column_and_compute(int col, int x, int y)
	{		
		init_column(col, x, y);
		return PASS_WINDOW_AS_PARAMETER ? 
			update_and_compute(col_values[col], skip_column(col, 1), win_pos_x, win_pos_y, win_width, win_height) :
			update_and_compute(col_values[col], skip_column(col, 1));
	}
	
	const col_value_type& update_column(int col, const pixel_type& new_pixel, const pixel_type& old_pixel)
	{
		ColValue& col_value = col_values[col];
		if (new_pixel != old_pixel)
		{
			if (DO_PIXEL_VALIDATION)
			{
				if (IsUseful(new_pixel))	col_value += new_pixel;
				if (IsUseful(old_pixel))	col_value -= old_pixel;
			}
			else
			{
				col_value += new_pixel;	col_value -= old_pixel;				
			}
		}
		return col_value;
	}

	void update_column_and_add(int col, const pixel_type& new_pixel, const pixel_type& old_pixel)
	{
		if (new_pixel == old_pixel) return;	// new_row and old_row value actually should be updated, however this can be saved since only their difference matters
		update_column(col, new_pixel, old_pixel);

		if (DO_PIXEL_VALIDATION)
		{
			if (IsUseful(new_pixel)) new_row_value += new_pixel;
			if (IsUseful(old_pixel)) old_row_value += old_pixel;
		}
		else
		{
			new_row_value += new_pixel;	old_row_value += old_pixel;
		}
	}
	
	const col_value_type& skip_column(int col, int dx) const	{	return col_values[(dx>0) ? col-WinSkip() : col+WinSkip()];	}

	// removing constness is useful in AccuHistArrayIncr::update_and_compute()
	// col_value_type& skip_column(int col, int dx)	{	return col_values[(dx>0) ? col-WinSkip() : col+WinSkip()];	}
	
	void before_go_next_row()	{	new_row_value.reset();	old_row_value.reset();	}
	fun_result_type new_row_init_compute()	
	{	
		return PASS_WINDOW_AS_PARAMETER ?
			update_and_compute(new_row_value, old_row_value, win_pos_x, win_pos_y, win_width, win_height) :
			update_and_compute(new_row_value, old_row_value);	
	}

	fun_result_type update_column_and_compute(int dx, int col, const pixel_type& new_pixel, const pixel_type& old_pixel)
	{		
		return PASS_WINDOW_AS_PARAMETER ? 
			update_and_compute(update_column(col, new_pixel, old_pixel), skip_column(col, dx), win_pos_x, win_pos_y, win_width, win_height) :
			update_and_compute(update_column(col, new_pixel, old_pixel), skip_column(col, dx));
	}
};

// TODO: PassWindowPara is not handled yet in scanline_sliding_window_calculator_coldiff
template<class InputImage, class AccuValue, class ColValue, class ColDiffValue, bool ValidatePixel>
class scanline_sliding_window_calculator_coldiff : public scanline_sliding_window_calculator<InputImage, AccuValue, ColValue, ValidatePixel, false>
{
	typedef scanline_sliding_window_calculator<InputImage, AccuValue, ColValue, ValidatePixel, false> Base;

protected:
	vector<ColDiffValue> col_diff_values;	// num_col-win_width column difference values, diff[n] = col_values[n+win_width] - col_values[n]
	ColDiffValue row_diff;					// new_row - old_row

public:
	static string ConfigName()	{	return DO_PIXEL_VALIDATION ? "ColDiffPV" : "ColDiff";	}

	scanline_sliding_window_calculator_coldiff(const unsigned int width, const unsigned int height
		,const unsigned int rwidth, const unsigned int rheight, const InputImage& img, bool column_wise)
		: Base(width, height, rwidth, rheight, img, column_wise), col_diff_values(col_values.size()-WinSkip()){}

	fun_result_type init_column_and_compute(int col, int x, int y)
	{
		init_column(col, x, y);
		col_diff_values[col-WinSkip()].create(col_values[col], col_values[col-WinSkip()]);
		return add_and_compute(col_diff_values[col-WinSkip()]);
	}

	void before_go_next_row()	{	row_diff.reset();	}

	void update_column(int col, const pixel_type& new_pixel, const pixel_type& old_pixel)
	{
		if (new_pixel != old_pixel)
		{
			// expression col - win_width >= 0 is problematic, since col is signed and win_width is unsigned, result is unsigned
			if (col >= static_cast<int>(WinSkip()))
			{
				ColDiffValue& diff_hist = col_diff_values[col-WinSkip()];
				if (DO_PIXEL_VALIDATION)
				{
					if (IsUseful(new_pixel))	diff_hist += new_pixel;
					if (IsUseful(old_pixel))	diff_hist -= old_pixel;
				}
				else 
				{
					diff_hist += new_pixel;
					diff_hist -= old_pixel;
				}
			}
			if (col+WinSkip() < col_values.size())
			{
				ColDiffValue& diff_hist = col_diff_values[col];
				if (DO_PIXEL_VALIDATION)
				{
					if (IsUseful(new_pixel))	diff_hist -= new_pixel;
					if (IsUseful(old_pixel))	diff_hist += old_pixel;
				}
				else
				{
					diff_hist -= new_pixel;	
					diff_hist += old_pixel;
				}
			}
		}
	}

	void update_column_and_add(int col, const pixel_type& new_pixel, const pixel_type& old_pixel)
	{
		if (new_pixel == old_pixel) return;
		update_column(col, new_pixel, old_pixel);
		if (DO_PIXEL_VALIDATION)
		{
			if (IsUseful(new_pixel)) row_diff += new_pixel;
			if (IsUseful(old_pixel)) row_diff -= old_pixel;
		}
		else
		{
			row_diff += new_pixel;	
			row_diff -= old_pixel;
		}
	}

	fun_result_type new_row_init_compute()	{	return add_and_compute(row_diff);	}

	fun_result_type update_column_and_compute(int dx, int col, const pixel_type& new_pixel, const pixel_type& old_pixel)
	{	
		update_column(col, new_pixel, old_pixel);
		return (dx>0) ? add_and_compute(col_diff_values[col-WinSkip()]) : remove_and_compute(col_diff_values[col]);
	}
};

// a template method for incremental calculation of a window sliding along scan lines
/*
range rect
                     range width
_______________________________________________________
|                                                     |
|                 search rect                         |
|            __________w__________                    |
|            |                    |                   |
|            |                    |                   | 
|            |      (cx, cy)      | h                 | range height
|            |                    |                   |
|            |____________________|                   |
|                                                     |
|                                                     |
|_____________________________________________________|

-> -> ...... -> ->
|
<- <- ...... <- <-
|
......

*/
template<class Calculator, class ResultWriter>
int incr_scanline_calculate(Calculator& calc, const int range_X, const int range_Y, ResultWriter& result)
{
	int num_update_compute = 0;			// #calls for update_and_compute()

	if (calc.is_column_wise)
	{
		// at left-top corner, init first w columns
		int x = range_X;
		calc.win_pos_x = range_X;
		calc.win_pos_y = range_Y;

		int dx = 1;	// increment of x coordinate, 1 or -1

		int col = 0;
		// initialize first w column values and compute result of initial window
		for(; col < static_cast<int>(calc.win_width); col++, x++)
			calc.init_column_and_add(col, x, calc.win_pos_y);
		result.init_write(static_cast<typename ResultWriter::value_type>(calc.init_compute()));

		// initialize remaining column histograms and compute remaining values in first row
		for(; col < static_cast<int>(calc.range_width); col++, x++, num_update_compute++) 
		{
			calc.win_pos_x++;	// update the window position before actual computation
			result.incrx_write(dx, static_cast<typename ResultWriter::value_type>(calc.init_column_and_compute(col, x, calc.win_pos_y)));
		}

		// row-by-row incremental calculation
		while (calc.win_pos_y+calc.win_height < range_Y+calc.range_height)
		{
			dx = -dx;		// invert the scanning direction
			calc.win_pos_y++;	// go to next row
			//printf("row %d\n", calc.win_pos_y);

			col = (dx > 0) ? 0 : calc.range_width-1;

			const Calculator::pixel_type* new_row_ptr = calc.input_image.RowPtr(calc.win_pos_y + calc.win_height-1);
			new_row_ptr += range_X+col;
			const Calculator::pixel_type* old_row_ptr = calc.input_image.RowPtr(calc.win_pos_y-1);
			old_row_ptr += range_X+col;

			// update current w column histograms
			calc.before_go_next_row();
			unsigned int counter = 0;
			for(; counter < calc.win_width; col+=dx, counter++, new_row_ptr+=dx, old_row_ptr+=dx)
				calc.update_column_and_add(col, *new_row_ptr, *old_row_ptr);

			num_update_compute++;
			result.incry_write(1, static_cast<typename ResultWriter::value_type>(calc.new_row_init_compute()));

			// sweep remaining columns
			for(; counter < calc.range_width; col+=dx, counter++, new_row_ptr+=dx, old_row_ptr+=dx, num_update_compute++)
			{
				calc.win_pos_x+=dx;	// update the window position before actual computation
				result.incrx_write(dx, static_cast<typename ResultWriter::value_type>(calc.update_column_and_compute(dx, col, *new_row_ptr, *old_row_ptr)));
			}
		}
	}
	else
	{
		// at left-top corner, init first h rows
		int y = range_Y;
		calc.win_pos_y = range_Y;
		calc.win_pos_x = range_X;

		int dy = 1;	// increment of y coordinate, 1 or -1

		int row = 0;
		// initialize first h row values and compute result of initial window
		for(; row < static_cast<int>(calc.win_height); row++, y++)
			calc.init_column_and_add(row, calc.win_pos_x, y);
		result.init_write(static_cast<typename ResultWriter::value_type>(calc.init_compute()));

		// initialize remaining row histograms and compute remaining values in first column
		for(; row < static_cast<int>(calc.range_height); row++, y++, num_update_compute++) 
		{
			calc.win_pos_y++;	// update the window position before actual computation
			result.incry_write(dy, static_cast<typename ResultWriter::value_type>(calc.init_column_and_compute(row, calc.win_pos_x, y)));
		}

		// column-by-column incremental calculation
		while (calc.win_pos_x+calc.win_width < range_X+calc.range_width)
		{
			dy = -dy;		// invert the scanning direction
			calc.win_pos_x++;	// go to next column
			//printf("column %d\n", calc.win_pos_x);

			row = (dy > 0) ? 0 : calc.range_height-1;

			const int new_col_x = calc.win_pos_x+calc.win_width-1;
			const int old_col_x = calc.win_pos_x-1;
			int new_col_y = range_Y + row;	
			const typename Calculator::input_image_type& img = calc.input_image;

			// update current h row histograms
			calc.before_go_next_row();
			unsigned int counter = 0;
			for(; counter < calc.win_height; row+=dy, counter++, new_col_y+=dy)
				calc.update_column_and_add(row, img.Pixel(new_col_x, new_col_y), img.Pixel(old_col_x, new_col_y));

			num_update_compute++;
			result.incrx_write(1, static_cast<typename ResultWriter::value_type>(calc.new_row_init_compute()));

			// sweep remaining columns
			for(; counter < calc.range_height; row+=dy, counter++, new_col_y+=dy, num_update_compute++)
			{
				calc.win_pos_y+=dy;	// update the window position before actual computation
				result.incry_write(dy, static_cast<typename ResultWriter::value_type>(calc.update_column_and_compute(dy, row, img.Pixel(new_col_x, new_col_y), img.Pixel(old_col_x, new_col_y))));
			}
		}
	}

	return num_update_compute * (calc.is_column_wise ? calc.win_height : calc.win_width);
}