

#pragma once

#include <opencv2/opencv.hpp>
#include <map>
#include <vector>
#include <math.h>
#include <windows.h>
using namespace std;
using namespace cv;

struct ImgWin
{
	cv::Rect box;
	int id;
	int parentId;
	int level;
};

typedef vector<vector<ImgWin>> WinSamps;


class Tools
{
public:

	static bool DrawWins(const Mat& color_img, vector<ImgWin>& wins)
	{
		cv::RNG rng(cv::getTickCount());
		cv::Mat dispimg = color_img.clone();
		for(size_t i=0; i<wins.size(); i++)
		{
			cv::rectangle(dispimg, wins[i].box, CV_RGB(rng.next()%255, rng.next()%500, rng.next()%255));
			cv::imshow("wins", dispimg);
			cv::waitKey(0);
		}

		return true;
	}

};