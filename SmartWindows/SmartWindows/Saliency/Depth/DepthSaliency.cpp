#include "DepthSaliency.h"


DepthSaliency::DepthSaliency(void)
{
}

//////////////////////////////////////////////////////////////////////////

double DepthSaliency::CompDepthVariance(const Mat& dmap, ImgWin win)
{
	// assume the dmap has been normalized to 0~255
	cv::Scalar mean, stddev;
	meanStdDev(dmap(win), mean, stddev);
	return (255-mean.val[0]) / (stddev.val[0]+0.000005f);
}

bool DepthSaliency::CompWinDepthSaliency(const Mat& dmap, ImgWin& win)
{
	// compute center-surround depth difference
	double sal = 0;
	cv::Scalar in_mean, in_stddev;
	meanStdDev(dmap(win), in_mean, in_stddev);
	sal = (255-in_mean.val[0]) / (in_stddev.val[0]+0.000001f);
	
	double context_sal = 0;
	Scalar out_mean, out_stddev;
	ImgWin contextWin = ToolFactory::GetContextWin(dmap.cols, dmap.rows, win, 1.5);
	meanStdDev(dmap(contextWin), out_mean, out_stddev);
	context_sal = (255-out_mean.val[0]) / (out_stddev.val[0]+0.000001f);

	double bordermean = (out_mean.val[0]*contextWin.area() - in_mean.val[0]*win.area()) / (contextWin.area()-win.area());
	win.score = fabs(bordermean-in_mean.val[0]) / in_stddev.val[0];
	win.tempvals.push_back(sal);
	win.tempvals.push_back(context_sal);

	return true;
}

void DepthSaliency::RankWins(const Mat& dmap, vector<ImgWin>& wins)
{
	// normalize
	Mat ndmap;
	normalize(dmap, ndmap, 0, 255, NORM_MINMAX);

	for (size_t i=0; i<wins.size(); i++)
	{
		CompWinDepthSaliency(ndmap, wins[i]);
	}

	sort(wins.begin(), wins.end(), [](const ImgWin& a, const ImgWin& b) { return a.score > b.score; } );

}