#include "VtlWrapper.h"
#include "segment-image.h"

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

	