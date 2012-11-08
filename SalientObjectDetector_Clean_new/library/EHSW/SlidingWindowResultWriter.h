#pragma once

template<class ValueType>
class ScanlineWriter
{
protected:
	int x, y;			// position to write result
	const int ix, iy;	// initial position

public:
	typedef ValueType value_type;
	ScanlineWriter(int initx, int inity) : x(initx), y(inity), ix(initx), iy(inity) {}
	void init_write(const value_type& v)	{	x=ix, y=iy;	}
	void incrx_write(int dx, const value_type& v)	{	x+=dx;	}
	void incry_write(int dy, const value_type& v)	{	y+=dy;	}
};

#include <functional>
// _Fn2_better is a binary_function that return true if arg1 is better than arg2
template<class _Ty, class _Fn2_better = less<_Ty> >
class OptimumFinderWriter : public ScanlineWriter<_Ty>
{
	typedef ScanlineWriter<_Ty> Base;

protected:
	_Fn2_better better;

public:
	value_type best_value;
	int best_x, best_y;

	OptimumFinderWriter(int initx, int inity) : Base(initx, inity), best_x(0), best_y(0), best_value(0) {}
	void init_write(const value_type& v)
	{
		x=ix, y=iy;
		best_value = v;
		best_x = x, best_y = y;
	}

	void incrx_write(int dx, const value_type& v)
	{
		x+=dx;
		if (better(v, best_value))
		{
			best_value = v;
			best_x = x;
			best_y = y;
		}
	}

	void incry_write(int dy, const value_type& v)
	{
		y+=dy;
		if (better(v, best_value))
		{
			best_value = v;
			best_x = x;
			best_y = y;
		}
	}
};

template<class Image, class BaseWriter = ScanlineWriter<typename Image::PixelType> >
class ImageResultWriter : public BaseWriter
{
	//typedef ScanlineWriter<typename Image::PixelType> Base;
	typedef BaseWriter Base;

public:
	typedef typename Base::value_type value_type;

	Image& img;
	ImageResultWriter(Image& i, int initx, int inity) : img(i), Base(initx, inity) {}
	void init_write(const value_type& v)
	{	
		Base::init_write(v);
		img.Pixel(x, y) = v;
	}

	void incrx_write(int dx, const value_type& v)
	{
		Base::incrx_write(dx, v);
		img.Pixel(x, y) = v;
	}

	void incry_write(int dy, const value_type& v)
	{	
		Base::incry_write(dy, v);
		img.Pixel(x, y) = v;
	}
};

#include <math.h>
#include <iostream>
// use ground truth for algorithm testing
template<class Image>
class ImageResultWriterGT : public ImageResultWriter<Image, ScanlineWriter<typename Image::PixelType> >
{
	//typedef ImageResultWriter<Image> Base;
	typedef ImageResultWriter<Image, ScanlineWriter<typename Image::PixelType> > Base;

	Image& ground_truth;		// ideally it should be const, but there may not be const Pixel() in OutputImage

	void write_result(const value_type& v)
	{
		value_type truth = ground_truth.Pixel(x, y);
		if (fabs((float)(truth - v)) > 1e-4)
			cerr << "wrong result value at " << x << " " << y << ", " << (float)v << " != " << (float)truth << endl;

		result.Pixel(x, y) = v;
	}

public:
	ImageResultWriterGT(Image& _result, Image& truth, int initx, int inity) : Base(_result, initx, inity), ground_truth(truth) {}
	void init_write(const value_type& v)	{	x=ix;	y=iy;	write_result(v);	}
	void incrx_write(int dx, const value_type& v) {	x+=dx;	write_result(v);	}
	void incry_write(int dy, const value_type& v) {	y+=dy;	write_result(v);	}
};

template<class Image1, class Image2, class BaseWriter = ScanlineWriter<pair<typename Image1::PixelType, typename Image2::PixelType> > >
class ImagePairResultWriter : public BaseWriter
{
	typedef BaseWriter Base;

public:
	typedef typename Base::value_type value_type;

	Image1& img1;
	Image2& img2;
	ImagePairResultWriter(Image1& i1, Image2& i2, int initx, int inity) : img1(i1), img2(i2), Base(initx, inity) {}
	void init_write(const value_type& v)
	{	
		Base::init_write(v);
		img1.Pixel(x, y) = v.first;
		img2.Pixel(x, y) = v.second;
	}

	void incrx_write(int dx, const value_type& v)
	{
		Base::incrx_write(dx, v);
		img1.Pixel(x, y) = v.first;
		img2.Pixel(x, y) = v.second;
	}

	void incry_write(int dy, const value_type& v)
	{	
		Base::incry_write(dy, v);
		img1.Pixel(x, y) = v.first;
		img2.Pixel(x, y) = v.second;
	}
};