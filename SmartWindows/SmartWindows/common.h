

#pragma once

#include <opencv2/opencv.hpp>
#include <map>
#include <vector>
#include <math.h>
using namespace std;
using namespace cv;

struct ImgWin
{
	cv::Rect box;
	int id;
	int parentId;
	int level;
};