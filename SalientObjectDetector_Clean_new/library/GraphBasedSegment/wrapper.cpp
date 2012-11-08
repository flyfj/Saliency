#include "wrapper.h"
#include "segment-image.h"

#include <assert.h>

int graph_based_segment(const BitmapData& bmp, float sigma, float c, int min_size, ImageUIntSimple& indexImg, unsigned char* segmentedImg)
{

	assert(bmp.PixelFormat == PixelFormat24bppRGB);
	
	//copy data to input
	const int width = bmp.Width;
	const int height = bmp.Height;
	image<rgb> input(width, height);
	for(int y = 0; y < height; y++) for(int x = 0; x < width; x++)
	{
		BYTE* pt = (BYTE*)bmp.Scan0 + y*bmp.Stride + x*3;
		rgb val;
		val.r = pt[2];
		val.b = pt[0];
		val.g = pt[1];
		input.access[y][x] = val;
	}

	image<int> index(width, height);	//index matrix, each pixel value is its object id (0~object_num)
	int num_ccs;
	image<rgb> *seg = segment_image(&input, sigma, c, min_size, &num_ccs, &index);	

	for(int y = 0; y < height; y++)	for(int x = 0; x < width; x++)
	{
		int loc = y*width*3 + 3*x;
		segmentedImg[loc] = imRef(seg,x,y).b;
		segmentedImg[loc+1] = imRef(seg,x,y).g;
		segmentedImg[loc+2] = imRef(seg,x,y).r;
	}

	delete seg;	

	indexImg.Create(width, height);
	for(int y = 0; y < height; y++) for(int x = 0; x < width; x++)
		indexImg.Pixel(x,y) = index.access[y][x];

	return num_ccs;

}

/*
int graph_based_segment(const CImageRgb& img, float sigma, float c, int min_size, CImage<unsigned int>& indexImg, CImageRgb& segmentedImg)
{	
	//copy data to input
	const int width = img.Width();
	const int height = img.Height();
	image<rgb> input(width, height);
	for(int y = 0; y < height; y++) for(int x = 0; x < width; x++)
	{
		PixelRgb t = img.Pixel(x,y);
		rgb val;
		val.r = t.R();
		val.b = t.B();
		val.g = t.G();
		input.access[y][x] = val;
	}
	
	image<int> index(width, height);	//index matrix, each pixel value is its object id (0~object_num)
	int num_ccs;
	image<rgb> *seg = segment_image(&input, sigma, c, min_size, &num_ccs, &index);	
	
	segmentedImg.Allocate(width, height);
	for(int y = 0; y < height; y++)	for(int x = 0; x < width; x++)
		segmentedImg.Pixel(x,y) = PixelRgb(imRef(seg,x,y).r, imRef(seg,x,y).g, imRef(seg,x,y).b);

	delete seg;	
	
	indexImg.Allocate(width, height);
	for(int y = 0; y < height; y++) for(int x = 0; x < width; x++)
		indexImg.Pixel(x,y) = index.access[y][x];

	return num_ccs;
}
*/