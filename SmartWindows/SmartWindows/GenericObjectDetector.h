//////////////////////////////////////////////////////////////////////////
// generic object detector
// jiefeng@2014-3-14
//////////////////////////////////////////////////////////////////////////

#pragma once

#include "common.h"
#include "ImgVisualizer.h"
//#include "WindowEvaluator.h"
//#include "ImageSpaceManager.h"
#include "DataManager/Berkeley3DDataManager.h"


class GenericObjectDetector
{
private:

	Mat Gx, Gy;
	Mat Gmag, Gdir;
	Mat integralGx, integralGy;
	Size imgSize;
	Mat depthMap;
	Mat img;

	cv::TermCriteria shiftCrit;

	Berkeley3DDataManager b3dmanager;



	bool ShiftWindow(const Point& seedPt, Size winSz, Point& newPt);

	double ComputeObjectScore(Rect win);

	double ComputeDepthVariance(Rect win);

public:
	GenericObjectDetector(void);

	bool Preprocess(const cv::Mat& color_img);

	bool Run(const cv::Mat& color_img);
	
	bool RunSlidingWin(const cv::Mat& color_img, Size winsz);

};

