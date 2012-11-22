
#include <opencv2\opencv.hpp>
#include <time.h>
#include <iostream>
#include <vector>
using namespace cv;

#include "BGMapExtractor_Grid.h"



int wmain(int argc, wchar_t *argv[])
{
	if (argc < 3)
	{
		printf("[Usage:]GS_Grid_Sample.exe source_image saliency_map_image\n");
		return -1;
	}

	Mat img = imread("photo.jpg");
	cv::cvtColor(img, img, CV_BGR2RGB);

	int w = img.cols;
	int h = img.rows;
	int stride = img.step;

	//initialize BGMap extractor
	//note that if we have multiple images to process, only need to initialize it once, 
	//but the iMaxWidth and iMaxHeight should be set to the maximum size of all images
	//for the grid size (iPatchWidth and iPatchHeight), usually if the image size is about 400*400, we can set the grid size to 10*10
	BGMapExtractor_Grid bgExtractor;
	int iMaxWidth = w;
	int iMaxHeight = h;
	int iPatchWidth = 10;
	int iPatchHeight = 10;

	double start_t = getTickCount();

	if (!bgExtractor.Initialize(iMaxWidth, iMaxHeight, iPatchWidth, iPatchHeight))
	{
		printf("bgExtractor initialization error.\n");
		return -1;
	}

	//allocate memory for saliency map
	float *pfBGMap = NULL;
	pfBGMap = new float[w * h];
	if (NULL == pfBGMap)
	{
		printf("memory error.\n");
		return -1;
	}
	//generate saliency map (saliency values have been normalized to 0-1)
	if (!bgExtractor.BGMapExtract((const unsigned char *)img.data, w, h, stride, pfBGMap))
	{
		printf("BG_map extraction error.\n");
		delete[] pfBGMap;
		return -1;
	}

	//save saliency map
	int sizes[] = {h,w};
	Mat mapimg(2, sizes, CV_32F, pfBGMap);
	mapimg.convertTo(mapimg, CV_32F, 255);

	imwrite("saliencymap.jpg", mapimg);

	double diff_t = getTickCount() - start_t;
	std::cout<<diff_t/getTickFrequency()<<" s"<<std::endl;

	delete[] pfBGMap;


	return 0;
}
