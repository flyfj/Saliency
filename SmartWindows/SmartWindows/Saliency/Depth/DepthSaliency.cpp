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

	//////////////////////////////////////////////////////////////////////////
	// do local normalization first
	double minval, maxval;
	minMaxLoc(dmap(contextWin), &minval, &maxval);
	// center-surround histogram
	int binnum = 30;
	int bin_step = 255 / binnum;
	Mat in_hist(1, binnum, CV_32F);
	in_hist.setTo(0);
	Mat out_hist(1, binnum, CV_32F);
	out_hist.setTo(0);
	for (int r=contextWin.tl().y; r<contextWin.br().y; r++)
	{
		for(int c=contextWin.tl().x; c<contextWin.br().x; c++)
		{
			float dval = dmap.at<float>(r, c);
			// normalize
			dval = dval*255 / maxval;
			int bin_id = MIN(binnum-1, (int)(dval/bin_step));
			if(win.contains(Point(c, r)))
				in_hist.at<float>(bin_id)++;
			else
				out_hist.at<float>(bin_id)++;
		}
	}

	normalize(in_hist, in_hist, 1, 0, NORM_L1);
	normalize(out_hist, out_hist, 1, 0, NORM_L1);
	win.score = compareHist(in_hist, out_hist, CV_COMP_BHATTACHARYYA);

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