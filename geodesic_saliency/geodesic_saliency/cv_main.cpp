
#include <opencv2\opencv.hpp>
#include <time.h>
#include <iostream>
#include <vector>
using namespace cv;

#include "BGMapExtractor_Grid.h"
#include "GeodesicSaliencyDetector.h"

using namespace Saliency;

int wmain(int argc, wchar_t *argv[])
{
	if (argc < 3)
	{
		printf("[Usage:]GS_Grid_Sample.exe source_image saliency_map_image\n");
		return -1;
	}

	GeodesicSaliencyDetector detector;

	Mat img = imread("photos.jpg");
	
	// init
	detector.Init(img.cols, img.rows, img.step);

	double start_t = cv::getTickCount();
	// detect
	Mat map;
	detector.ComputeSaliencyMap(img, map);

	imshow("sal", map);
	waitKey(0);

	double delth_t = (cv::getTickCount() - start_t) / cv::getTickFrequency();
	std::cout<<"Processing time: "<<delth_t<<std::endl;

	getchar();

	return 0;
}
