

//#include "ImageSpaceManager.h"
//#include "WindowEvaluator.h"
#include "GenericObjectDetector.h"
#include "ShapeAnalyzer.h"
#include "ImgVisualizer.h"
#include "DataManager/Berkeley3DDataManager.h"
#include <string>
using namespace std;

int main()
{
	std::string imgdir = "E:\\Datasets\\VOC2007\\VOCtrainval_06-Nov-2007\\VOC2007\\JPEGImages\\";
	std::string imgname = "000019";

	ShapeAnalyzer shaper;
	GenericObjectDetector detector;
	Berkeley3DDataManager berkeleyManager;
	
	// load image
	FileInfos imgs;
	berkeleyManager.GetImageList(imgs);
	cv::Mat curimg = imread(imgs[0].filepath);
	imshow("img", curimg);
	cv::waitKey(10);
	//cv::Mat img = cv::imread("tiger.jpg");
	//cvtColor(img, img, CV_BGR2GRAY);

	// process
	double start_t = cv::getTickCount();
	
	detector.Run(curimg);

	std::cout<<"Process time: "<<(cv::getTickCount()-start_t) / cv::getTickFrequency()<<"s."<<std::endl;


	cv::waitKey(0);
	cv::destroyAllWindows();

	return 0;
}