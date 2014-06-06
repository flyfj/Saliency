

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
	dbMan.Init(DB_VOC07);
	visualsearch::ImageSegmentor segmentor;

	// process
	if( !detector.InitBingObjectness() )
		return -1;

	Mat timg = imread("d:\\imgs\\img_0264.png");
	if(timg.empty())
		return 0;
	//resize(timg, timg, Size(300,300));
	//Mat normimg;
	//normalize(timg, timg, 0, 255, NORM_MINMAX);

	double start_t = cv::getTickCount();

	vector<Rect> boxes;
	detector.GetObjectsFromBing(timg, boxes, 120, true);

	//detector.test();
	//dbMan.BrowseDBImages();
	//dbMan.GenerateWinSamps();
	//detector.Run(curimg);

	std::cout<<"Process time: "<<(cv::getTickCount()-start_t) / cv::getTickFrequency()<<"s."<<std::endl;

	cv::waitKey(0);
	cv::destroyAllWindows();

	getchar();

	return 0;
}