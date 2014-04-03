
#include "GenericObjectDetector.h"



GenericObjectDetector::GenericObjectDetector(void)
{
	shiftCrit.maxCount = 10;
	shiftCrit.epsilon = 0.0001f;
}

//////////////////////////////////////////////////////////////////////////

bool GenericObjectDetector::Preprocess(const cv::Mat& color_img)
{
	imgSize.width = color_img.cols;
	imgSize.height = color_img.rows;

	// convert to gray
	Mat grayimg;
	cv::cvtColor(color_img, grayimg, CV_BGR2GRAY);

	// compute gradient
	Sobel(grayimg, Gx, CV_32F, 1, 0, 3);
	double maxv, minv;
	minMaxLoc(Gx, &minv, &maxv);
	Sobel(grayimg, Gy, CV_32F, 0, 1, 3);
	ImgVisualizer::DrawFloatImg("gx", Gx, Mat());
	ImgVisualizer::DrawFloatImg("gy", Gy, Mat());

	// magnitude
	magnitude(Gx, Gy, Gmag);
	ImgVisualizer::DrawFloatImg("gmag", Gmag, Mat());

	phase(Gx, Gy, Gdir, true);

	// compute integrals for gx, gy
	cv::integral(Gx, integralGx, CV_64F);
	cv::integral(Gy, integralGy, CV_64F);


	// prepare data
	FileInfos dmaps;
	db_man.GetDepthmapList(dmaps);
	db_man.LoadDepthData(dmaps[0].filepath, depthMap);

	return true;
}

double GenericObjectDetector::ComputeObjectScore(Rect win)
{
	// compute edge orientation difference sum
	double sumgx = integralGx.at<double>(win.br().y, win.br().x);
	sumgx += integralGx.at<double>(win.tl().y, win.tl().x);
	sumgx -= integralGx.at<double>(win.tl().y, win.br().x);
	sumgx -= integralGx.at<double>(win.br().y, win.tl().x);

	double sumgy = integralGy.at<double>(win.br().y, win.br().x);
	sumgy += integralGy.at<double>(win.tl().y, win.tl().x);
	sumgy -= integralGy.at<double>(win.tl().y, win.br().x);
	sumgy -= integralGy.at<double>(win.br().y, win.tl().x);

	return 1 / sqrt(sumgx*sumgx + sumgy*sumgy);
}

double GenericObjectDetector::ComputeDepthVariance(Rect win)
{
	cv::Scalar mean, stddev;
	meanStdDev(depthMap(win), mean, stddev);
	return 1.f / (stddev.val[0]+0.000005f);
}

//////////////////////////////////////////////////////////////////////////

bool GenericObjectDetector::test()
{
	// test oversegmentation
	// get one image and depth map
	FileInfos imglist;
	db_man.GetImageList(imglist);
	img = imread(imglist[0].filepath);
	db_man.GetDepthmapList(imglist);
	db_man.LoadDepthData(imglist[0].filepath, depthMap);

	vector<visualsearch::SuperPixel> sps;
	segmentor.ExtractRGBDSuperpixel(img, depthMap, sps);
	// visualize


	return true;
}

bool GenericObjectDetector::ShiftWindow(const Point& seedPt, Size winSz, Point& newPt)
{
	// define window
	Point topleft(seedPt.x-winSz.width/2, seedPt.y-winSz.height/2);
	topleft.x = MAX(0, topleft.x);
	topleft.y = MAX(0, topleft.y);

	Point bottomright(seedPt.x+winSz.width/2, seedPt.y+winSz.height/2);
	bottomright.x = MIN(bottomright.x, imgSize.width-1);
	bottomright.y = MIN(bottomright.y, imgSize.height-1);

	Rect win(topleft, bottomright);

	// start iteration
	for(int i=0; i<shiftCrit.maxCount; i++)
	{
		// compute edge orientation difference sum
		double sumgx = integralGx.at<double>(win.br().y, win.br().x);
		sumgx += integralGx.at<double>(win.tl().y, win.tl().x);
		sumgx -= integralGx.at<double>(win.tl().y, win.br().x);
		sumgx -= integralGx.at<double>(win.br().y, win.tl().x);

		double sumgy = integralGy.at<double>(win.br().y, win.br().x);
		sumgy += integralGy.at<double>(win.tl().y, win.tl().x);
		sumgy -= integralGy.at<double>(win.tl().y, win.br().x);
		sumgy -= integralGy.at<double>(win.br().y, win.tl().x);


	}

	return true;
}

bool GenericObjectDetector::RunSlidingWin(const cv::Mat& color_img, Size winsz)
{
	if(winsz.width > imgSize.width || winsz.height > imgSize.height)
		return false;

	cout<<"Running sliding window detection..."<<endl;

	cv::Mat scoremap(imgSize.height, imgSize.width, CV_64F);
	scoremap.setTo(0);

	vector<ImgWin> wins;

	for(int r=0; r<imgSize.height-winsz.height-1; r++)
	{
		for(int c=0; c<imgSize.width-winsz.width-1; c++)
		{
			ImgWin curwin(c, r, winsz.width, winsz.height);
			curwin.score = ComputeDepthVariance(curwin); //ComputeObjectScore(curwin);
			scoremap.at<double>(r+winsz.height/2, c+winsz.width/2) = curwin.score;

			wins.push_back(curwin);
		}
	}

	sort(wins.begin(), wins.end());

	ImgVisualizer::DrawFloatImg("scoremap", scoremap, Mat());
	ImgVisualizer::DrawImgWins(color_img, wins);
	waitKey(10);

	return true;
}

bool GenericObjectDetector::Run(const cv::Mat& color_img)
{
	if( !Preprocess(color_img) )
		return false;

	// test with simple sliding window
	RunSlidingWin(color_img, Size(100, 100));


	return true;
}