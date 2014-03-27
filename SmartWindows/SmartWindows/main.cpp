

//#include "ImageSpaceManager.h"
//#include "WindowEvaluator.h"
#include "GenericObjectDetector.h"
#include "ShapeAnalyzer.h"
#include "ImgVisualizer.h"
#include <string>
using namespace std;

int main()
{
	std::string imgdir = "E:\\Datasets\\VOC2007\\VOCtrainval_06-Nov-2007\\VOC2007\\JPEGImages\\";
	std::string imgname = "000019";

	// load image
	cv::Mat img = cv::imread("tiger.jpg");
	cv::imshow("input", img);
	cv::waitKey(10);

	// process
	double start_t = cv::getTickCount();
	
	ShapeAnalyzer shaper;
	GenericObjectDetector detector;

	vector<BasicShape> shapes;
	shaper.ExtractShapes(img, 0.05, CV_RETR_TREE, shapes);
	ImgVisualizer::DrawShapes(img, shapes);
	//detector.Run(img);

	std::cout<<"Process time: "<<(cv::getTickCount()-start_t) / cv::getTickFrequency()<<"s."<<std::endl;


	cv::waitKey(0);
	cv::destroyAllWindows();

	return 0;
}