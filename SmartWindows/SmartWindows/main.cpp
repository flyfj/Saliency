

//#include "ImageSpaceManager.h"
//#include "WindowEvaluator.h"
#include "GenericObjectDetector.h"
#include "ShapeAnalyzer.h"
#include "ImgVisualizer.h"
#include "DataManager/DatasetManager.h"
#include <string>
using namespace std;

int main()
{
	std::string imgdir = "E:\\Datasets\\VOC2007\\VOCtrainval_06-Nov-2007\\VOC2007\\JPEGImages\\";
	std::string imgname = "000019";

	ShapeAnalyzer shaper;
	GenericObjectDetector detector;
	DatasetManager dbMan(DB_VOC07);
	

	// process
	double start_t = cv::getTickCount();
	
	dbMan.BrowseDBImages();
	//detector.Run(curimg);

	std::cout<<"Process time: "<<(cv::getTickCount()-start_t) / cv::getTickFrequency()<<"s."<<std::endl;


	cv::waitKey(0);
	cv::destroyAllWindows();

	return 0;
}