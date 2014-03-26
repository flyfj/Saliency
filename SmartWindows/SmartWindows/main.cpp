

//#include "ImageSpaceManager.h"
//#include "WindowEvaluator.h"
#include "GenericObjectDetector.h"
#include <string>
using namespace std;

int main()
{
	std::string imgdir = "E:\\Datasets\\VOC2007\\VOCtrainval_06-Nov-2007\\VOC2007\\JPEGImages\\";
	std::string imgname = "000019";

	// load image
	cv::Mat img = cv::imread(imgdir + imgname + ".jpg");
	cv::imshow("input", img);
	cv::waitKey(10);

	// process
	double start_t = cv::getTickCount();
	
	GenericObjectDetector detector;

	detector.Run(img);

	std::cout<<"Process time: "<<(cv::getTickCount()-start_t) / cv::getTickFrequency()<<"s."<<std::endl;


	cv::waitKey(0);
	cv::destroyAllWindows();

	return 0;
}