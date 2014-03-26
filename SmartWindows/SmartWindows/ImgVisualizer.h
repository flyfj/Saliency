//////////////////////////////////////////////////////////////////////////
// a class for visualization functions
// jiefeng@2014-3-26
//////////////////////////////////////////////////////////////////////////


#pragma once

#include "common.h"


class ImgVisualizer
{
private:



public:
	ImgVisualizer(void);

	// draw image windows
	static bool DrawImgWins(const Mat& img, const vector<ImgWin>& wins);

	// visualize float precision image
	static bool DrawFloatImg(string winname, const Mat& img, Mat& oimg, bool toDraw = true);

};

