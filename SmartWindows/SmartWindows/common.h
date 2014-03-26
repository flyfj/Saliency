

#pragma once

#include <opencv2/opencv.hpp>
#include <map>
#include <vector>
#include <math.h>
#include <windows.h>
using namespace std;
using namespace cv;

class Win: public Rect
{
public:
	Win() {}
	Win(int x, int y, int w, int h): Rect(x, y, w, h) {}
};

class ImgWin: public Win
{
public:
	ImgWin(): Win() {}
	ImgWin(int x, int y, int w, int h): Win(x, y, w, h){}
	double score;

	inline bool operator < (const ImgWin& rwin) const
	{
		return score < rwin.score;
	}
};



typedef vector<vector<ImgWin>> WinSamps;

