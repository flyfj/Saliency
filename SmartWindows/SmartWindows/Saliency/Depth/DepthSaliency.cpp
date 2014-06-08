#include "DepthSaliency.h"


DepthSaliency::DepthSaliency(void)
{
}

//////////////////////////////////////////////////////////////////////////

double DepthSaliency::CompDepthVariance(const Mat& dmap, ImgWin win)
{
	cv::Scalar mean, stddev;
	meanStdDev(dmap(win), mean, stddev);
	return 1.f / (stddev.val[0]+0.000005f);
}

void DepthSaliency::RankWins(const Mat& dmap, vector<ImgWin>& wins)
{
	for (size_t i=0; i<wins.size(); i++)
	{
		wins[i].score = CompDepthVariance(dmap, wins[i]);
	}

	sort(wins.begin(), wins.end(), [](const ImgWin& a, const ImgWin& b) { return a.score < b.score; } );

}