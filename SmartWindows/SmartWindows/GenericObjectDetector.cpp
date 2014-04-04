
#include "GenericObjectDetector.h"



GenericObjectDetector::GenericObjectDetector(void)
{
	shiftCrit.maxCount = 10;
	shiftCrit.epsilon = 0.0001f;

	// detection window size
	winconfs.push_back(WinConfig(100, 100));
	winconfs.push_back(WinConfig(200, 200));
	winconfs.push_back(WinConfig(100, 200));
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
	/*ImgVisualizer::DrawFloatImg("gx", Gx, Mat());
	ImgVisualizer::DrawFloatImg("gy", Gy, Mat());*/

	// magnitude
	magnitude(Gx, Gy, Gmag);
	ImgVisualizer::DrawFloatImg("gmag", Gmag, Mat());

	phase(Gx, Gy, Gdir, true);

	// compute integrals for gx, gy
	cv::integral(Gx, integralGx, CV_64F);
	cv::integral(Gy, integralGy, CV_64F);

	// compute color integrals
	cv::Mat labImg;
	cv::cvtColor(color_img, labImg, CV_BGR2Lab);

	// split channels
	std::vector<cv::Mat> colorChannels(3);
	cv::split(labImg, colorChannels);

	// compute integrals
	colorIntegrals.resize(3);
	for(int i=0; i<3; i++) cv::integral(colorChannels[i], colorIntegrals[i], CV_64F);


	// prepare data
	/*FileInfos dmaps;
	db_man.GetDepthmapList(dmaps);
	db_man.LoadDepthData(dmaps[0].filepath, depthMap);*/

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

double GenericObjectDetector::ComputeCenterSurroundMeanColorDiff(ImgWin win)
{
	// context window
	int minx = MAX(win.x - win.width/2, 0);
	int miny = MAX(win.y - win.height/2, 0);
	int maxx = MIN(win.br().x + win.width/2, imgSize.width-1);
	int maxy = MIN(win.br().y + win.height/2, imgSize.height-1);
	Rect contextWin(minx, miny, maxx-minx, maxy-miny);

	// center mean
	Scalar centerMean, contextMean;
	for (int i=0; i<3; i++) 
	{
		centerMean.val[i] = ToolFactory::GetIntegralValue(colorIntegrals[i], win);
		contextMean.val[i] = ToolFactory::GetIntegralValue(colorIntegrals[i], contextWin);
		contextMean.val[i] -= centerMean.val[i];

		// average
		centerMean.val[i] /= win.area();
		contextMean.val[i] /= (contextWin.area() - win.area());
	}

	double score = 0;
	for(int i=0; i<3; i++) score += (centerMean.val[i]-contextMean.val[i])*(centerMean.val[i]-contextMean.val[i]);
	score = sqrt(score);

	return score;

}

//////////////////////////////////////////////////////////////////////////

bool GenericObjectDetector::WinCenterRange(const Rect spbox, const WinConfig winconf, Point& minPt, Point& maxPt)
{
	if(spbox.area() >= winconf.width*winconf.height)
	{
		cerr<<"Detection window is smaller than superpixel box."<<endl;
		return false;
	}

	minPt.x = spbox.br().x - winconf.width / 2;
	minPt.y = spbox.br().y - winconf.height / 2;
	minPt.x = MAX(minPt.x, winconf.width/2);
	minPt.y = MAX(minPt.y, winconf.height/2);
	maxPt.x = spbox.x + winconf.width / 2;
	maxPt.y = spbox.y + winconf.height / 2;
	maxPt.x = MIN(maxPt.x, imgSize.width-winconf.width/2-1);
	maxPt.y = MIN(maxPt.y, imgSize.height-winconf.height/2-1);

	minPt.x = MIN(minPt.x, maxPt.x);
	minPt.y = MIN(minPt.y, maxPt.y);
	maxPt.x = MAX(minPt.x, maxPt.x);
	maxPt.y = MAX(minPt.y, maxPt.y);

	cout<<spbox.x+spbox.width/2<<" "<<spbox.y+spbox.height/2<<endl;
	cout<<minPt.x<<" "<<minPt.y<<" "<<maxPt.x<<" "<<maxPt.y<<endl;

	assert(minPt.x <= maxPt.x && minPt.y <= maxPt.y);

	return true;
}

bool GenericObjectDetector::SampleWinLocs(const Point startPt, const WinConfig winconf, const Point minPt, const Point maxPt, int num, vector<ImgWin>& wins)
{
	/*if(startPt.x < minPt.x || startPt.x > maxPt.x || startPt.y < minPt.y || startPt.y > maxPt.y)
	{
	cerr<<"start point is not in the valid range."<<endl;
	return false;
	}*/

	// x range; y range
	// restrict to move in a bounding box
	double minx = startPt.x - minPt.x;
	double maxx = maxPt.x - startPt.x;
	double xrange = MIN(minx, maxx);
	double miny = startPt.y - minPt.y;
	double maxy = maxPt.y - startPt.y;
	double yrange = MIN(miny, maxy);

	cv::RNG rng;
	// generate randomly point within the range
	wins.resize(num);
	for(size_t i=0; i<num; i++)
	{
		// select a new center
		double xdiff = rng.uniform(-xrange, xrange);
		double ydiff = rng.uniform(-yrange, yrange);
		wins[i].x = startPt.x + xdiff - winconf.width/2;
		wins[i].y = startPt.y + ydiff - winconf.height/2;
		wins[i].width = winconf.width;
		wins[i].height = winconf.height;
	}

	return true;
}


//////////////////////////////////////////////////////////////////////////

bool GenericObjectDetector::test()
{
	// test oversegmentation
	// get one image and depth map
	FileInfos imglist;
	db_man.Init(DB_VOC07);
	if( !db_man.GetImageList(imglist) )
		return false;
	img = imread(imglist[0].filepath);
	//db_man.GetDepthmapList(imglist);
	//db_man.LoadDepthData(imglist[0].filepath, depthMap);

	// preprocessing
	Preprocess(img);

	segmentor.DoSegmentation(img);
	const vector<visualsearch::SuperPixel>& sps = segmentor.superPixels;

	int cnt = 10;
	while(cnt >= 0)
	{
		// select a segment
		srand(time(NULL));
		int sel_id = rand()%sps.size();
		vector<ImgWin> imgwins;
		// place window at center
		ImgWin curwin;
		curwin.x = sps[sel_id].box.x+sps[sel_id].box.width/2 - winconfs[0].width/2;
		curwin.y = sps[sel_id].box.y+sps[sel_id].box.height/2 - winconfs[0].height/2;
		curwin.width = winconfs[0].width;
		curwin.height = winconfs[0].height;
		imgwins.push_back(curwin);
		curwin = ImgWin(sps[sel_id].box.x, sps[sel_id].box.y, sps[sel_id].box.width, sps[sel_id].box.height);
		imgwins.push_back(curwin);
		ImgVisualizer::DrawImgWins("img", img, imgwins);
		ImgVisualizer::DrawImgWins("seg", segmentor.m_segImg, imgwins);
		ImgVisualizer::DrawImgWins("meancolor", segmentor.m_mean_img, imgwins);

		// compute adjust range
		Point minpt, maxpt;
		if( !WinCenterRange(sps[sel_id].box, winconfs[0], minpt, maxpt) )
			continue;

		// check range
		//ImgWin minWin(minpt.x-winconfs[0].width/2, minpt.y-winconfs[0].height/2, winconfs[])

		Point curpt(sps[sel_id].box.x+sps[sel_id].box.width/2, sps[sel_id].box.y+sps[sel_id].box.height/2);
		// do shifting
		for(int i=0; i<10; i++)
		{
			// generate locations
			vector<ImgWin> wins;
			SampleWinLocs(curpt, winconfs[0], minpt, maxpt, 6, wins);
			for(size_t j=0; j<wins.size(); j++)
				wins[j].score = ComputeCenterSurroundMeanColorDiff(wins[j]);

			// sort
			sort(wins.begin(), wins.end());

			// shift to max point
			const ImgWin& bestWin = wins[wins.size()-1];
			cout<<"Best score: "<<bestWin.score<<endl;
			curpt.x = bestWin.x + bestWin.width/2;
			curpt.y = bestWin.y + bestWin.height/2;

			// visualize
			vector<ImgWin> imgwins2;
			ImgWin spwin = ImgWin(sps[sel_id].box.x, sps[sel_id].box.y, sps[sel_id].box.width, sps[sel_id].box.height);
			imgwins2.push_back(spwin);
			imgwins2.push_back(bestWin);
			ImgVisualizer::DrawImgWins("shift", img, imgwins2);
			cv::waitKey(0);
			cv::destroyWindow("shift");
		}

		cnt--;
	}

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

bool GenericObjectDetector::ShiftWindowToMaxScore(const Point& seedPt, Point& newPt)
{
	// sample points


	// compute score

	// shift to max point

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
	ImgVisualizer::DrawImgWins("img", color_img, wins);
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