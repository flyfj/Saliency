

//#include "ImageSpaceManager.h"
//#include "WindowEvaluator.h"
#include "GenericObjectDetector.h"
#include "ShapeAnalyzer.h"
#include "ImgVisualizer.h"
#include "DataManager/DatasetManager.h"
#include <string>
#include "ObjectSegmentor.h"
#include "a9wins/A9Window.h"
using namespace std;

int main()
{
	ShapeAnalyzer shaper;
	GenericObjectDetector detector;
	DatasetManager dbMan;
	dbMan.Init(DB_BERKELEY3D);
	visualsearch::ImageSegmentor segmentor;

	// process
	double start_t = cv::getTickCount();
	
	//detector.test();
	//dbMan.BrowseDBImages();
	dbMan.GenerateWinSamps();
	//detector.Run(curimg);

	std::cout<<"Process time: "<<(cv::getTickCount()-start_t) / cv::getTickFrequency()<<"s."<<std::endl;

	cv::waitKey(0);
	cv::destroyAllWindows();

	return 0;
}