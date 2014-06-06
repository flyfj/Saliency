

//#include "ImageSpaceManager.h"
//#include "WindowEvaluator.h"
#include "GenericObjectDetector.h"
#include "ShapeAnalyzer.h"
#include "ImgVisualizer.h"
#include "DataManager/DatasetManager.h"
#include <string>
#include "ObjectSegmentor.h"
#include "a9wins/A9Window.h"
#include "Saliency/Composition/SalientRegionDetector.h"
using namespace std;

int main()
{
	ShapeAnalyzer shaper;
	GenericObjectDetector detector;
	DatasetManager dbMan;
	dbMan.Init(DB_VOC07);
	visualsearch::ImageSegmentor segmentor;

	// process
	if( !detector.InitBingObjectness() )
		return -1;

	Mat timg = imread("d:\\imgs\\a3.jpg");
	if(timg.empty())
		return 0;

	resize(timg, timg, Size(200,200));
	imshow("input img", timg);
	waitKey(10);
	//Mat normimg;
	//normalize(timg, timg, 0, 255, NORM_MINMAX);

	double start_t = cv::getTickCount();

	vector<ImgWin> boxes;
	detector.GetObjectsFromBing(timg, boxes, 200);

	std::cout<<"Bing time: "<<(cv::getTickCount()-start_t) / cv::getTickFrequency()<<"s."<<std::endl;
	
	// make images
	vector<Mat> imgs(boxes.size());
	for (int i=0; i<boxes.size(); i++)
	{
		imgs[i] = timg(boxes[i]);
	}

	Mat dispimg;
	visualsearch::ImgVisualizer::DrawImgCollection("objectness", imgs, 15, dispimg);
	imshow("objectness", dispimg);
	waitKey(10);
	
	// rank windows with CC
	
	SalientRegionDetector salDetector;
	salDetector.Init(timg);

	start_t = getTickCount();
	salDetector.RankWins(boxes);

	std::cout<<"Saliency time: "<<(cv::getTickCount()-start_t) / cv::getTickFrequency()<<"s."<<std::endl;

	// make images
	for (int i=0; i<boxes.size(); i++)
	{
		imgs[i] = timg(boxes[i]);
	}

	visualsearch::ImgVisualizer::DrawImgCollection("objectness", imgs, 15, dispimg);
	imshow("rank by CC", dispimg);
	waitKey(0);

	cv::destroyAllWindows();

	//getchar();

	return 0;
}