#pragma once

//////////////////////////////////////////////////////////////////////////
// concepts used in SlidingWindowCalculator
template<class Pixel>
class InputImageDummy
{
public:
	typedef Pixel PixelType;
	PixelType p;

	const PixelType* RowPtr(int row) const	{ return static_cast<const PixelType*>(0); }
	const PixelType& Pixel(int x, int y) const 
	{		
		return p;
	}
};

// accumulated value of a column(row)
class ColValueDummy
{
public:
	// used when initialized, e.g., init a column and advance a row
	void reset() {}

	template<class PixelType>
	void operator+=(const PixelType& p)	{}

	template<class PixelType>
	void operator-=(const PixelType& p)	{}
};

// difference of two column(row) values
class ColDiffValueDummy
{
public:
	void create(const ColValueDummy& add, const ColValueDummy& remove){}

	// used when initialized, e.g., init a column and advance a row
	void reset() {}

	template<class PixelType>
	void operator+=(const PixelType& p)	{}

	template<class PixelType>
	void operator-=(const PixelType& p)	{}
};

// accumulated value of the window
template<class PixelType>
class AccuValueDummy
{
public:
	typedef int fun_result_type;

	// an accumulation type usually needs an initialization given total pixel number
	AccuValueDummy(unsigned int PixelCount){}

	void operator+=(const ColValueDummy& c)	{}

	fun_result_type init_compute() {	return 0;	}
	fun_result_type init_compute(unsigned int win_x, unsigned int win_y, unsigned int win_width, unsigned int win_height) {	return 0;	}

	// for scanline_sliding_window_calculator_col
	fun_result_type update_and_compute(const ColValueDummy& add, const ColValueDummy& remove)	
	{	
		return 0;
	}

	// a wrapper that takes window as parameters
	fun_result_type update_and_compute(const ColValueDummy& add, const ColValueDummy& remove
		,unsigned int win_x, unsigned int win_y, unsigned int win_width, unsigned int win_height)
	{	
		return 0;	
	}

	// for scanline_sliding_window_calculator_coldiff
	fun_result_type add_and_compute(const ColDiffValueDummy& diff)	{	return 0;	}
	fun_result_type remove_and_compute(const ColDiffValueDummy& diff)	{	return 0;	}

	bool IsUseful(const PixelType& p) const	{	return true;	}
};

template<class ValueType>
class ResultWriterDummy
{
public:
	typedef ValueType value_type;
	void init_write(const ValueType& v) {}
	void incrx_write(int dx, const ValueType& v) {}
	void incry_write(int dy, const ValueType& v) {}
};

template<class Pixel>
class OutputImageDummy
{
public:
	typedef Pixel PixelType;
	PixelType p;

	vector<vector<PixelType>> data;

	OutputImageDummy(){}
	OutputImageDummy(unsigned int w, unsigned int h) : data(h, vector<PixelType>(w)){}

	PixelType& Pixel(int x, int y)
	{ 		
		if (data.empty()) return p;
		else return data[y][x];
	}
};