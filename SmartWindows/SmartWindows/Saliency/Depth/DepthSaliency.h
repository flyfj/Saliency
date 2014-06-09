//////////////////////////////////////////////////////////////////////////
// measure saliency of a window using depth
// jiefeng©2014-6-7
//////////////////////////////////////////////////////////////////////////

#pragma once

#include "common.h"
#include "Tools.h"

class DepthSaliency
{
private:


public:
	DepthSaliency(void);

	double CompDepthVariance(const Mat& dmap, ImgWin win);

	bool CompWinDepthSaliency(const Mat& dmap, ImgWin& win);

	void RankWins(const Mat& dmap, vector<ImgWin>& wins);
};

