//////////////////////////////////////////////////////////////////////////
// manage image space
// jiefeng@2014-03-08
//////////////////////////////////////////////////////////////////////////


#pragma once

#include <opencv2\opencv.hpp>
#include <map>
#include <vector>
#include <math.h>


enum DivideCriteria
{
	DIV_MEANCOLORDIFF
};

struct ImgWin
{
	cv::Rect box;
	int id;
	int parentId;
	int level;
};



class ImageSpaceManager
{
private:
	std::vector<cv::Mat> colorIntegrals;
	float minWinArea;
	int maxLevel;
	DivideCriteria crit;

	double GetIntegralValue(const cv::Mat& integralImg, cv::Rect box);

public:

	std::vector<ImgWin> wins;

	ImageSpaceManager(void);

	bool Preprocess(cv::Mat& color_img);

	// compute integral images for each color channels
	bool ComputeColorIntegrals(const cv::Mat& color_img);

	bool DivideImage(cv::Mat& color_img);

	// recursively divide image window based separation criteria
	bool Divide(ImgWin& rootWin);

	// draw generated windows
	bool DrawWins(const cv::Mat& color_img, std::vector<ImgWin>& allwins);

};

