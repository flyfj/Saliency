#include "ImgVisualizer.h"


ImgVisualizer::ImgVisualizer(void)
{
}

//////////////////////////////////////////////////////////////////////////

bool ImgVisualizer::DrawImgWins(const Mat& img, const vector<ImgWin>& wins)
{
	cv::RNG rng(cv::getTickCount());
	cv::Mat dispimg;
	if(img.channels() == 3)
		dispimg = img.clone();
	else
	{
		DrawFloatImg("", img, dispimg, false);
		cvtColor(dispimg, dispimg, CV_GRAY2BGR);
	}

	for(size_t i=wins.size()-1; i>=wins.size()-20; i--)
	{
		cv::rectangle(dispimg, wins[i], CV_RGB(rng.next()%255, rng.next()%255, rng.next()%255), 2);
	}
	cv::imshow("wins", dispimg);
	cv::waitKey(10);

	return true;
}

bool ImgVisualizer::DrawFloatImg(string winname, const Mat& img, Mat& oimg, bool toDraw)
{
	// normalize
	cv::normalize(img, oimg, 1, 0, NORM_MINMAX);
	// convert to 8u
	oimg.convertTo(oimg, CV_8U, 255);

	if( toDraw )
	{
		// show image
		imshow(winname, oimg);
		cv::waitKey(10);
	}

	return true;
}
