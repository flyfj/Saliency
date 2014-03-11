

#include "ImageSpaceManager.h"
#include "WindowEvaluator.h"
#include <string>
using namespace std;

int main()
{
	std::string imgdir = "E:\\Datasets\\VOC2007\\VOCtrainval_06-Nov-2007\\VOC2007\\JPEGImages\\";
	std::string imgfile = imgdir + "000012.jpg";
	string gtfile = "E:\\Datasets\\VOC2007\\VOC2007_AnnotationsOpenCV_Readable\\";
	gtfile += "000012.yml";

	// load image
	cv::Mat img = cv::imread(imgfile);
	cv::imshow("input", img);
	cv::waitKey(10);

	// load ground truth
	std::vector<ImgWin> gt_wins;
	WindowEvaluator eval;
	eval.LoadVOC07Boxes(gtfile, gt_wins);

	ImageSpaceManager imgSpaceManager;
	imgSpaceManager.DrawWins(img, gt_wins);
	cv::imshow("gt", img);
	cv::waitKey(0);

	// process
	double start_t = cv::getTickCount();
	
	imgSpaceManager.DivideImage(img);

	std::cout<<"Process time: "<<(cv::getTickCount()-start_t) / cv::getTickFrequency()<<"s."<<std::endl;

	imgSpaceManager.DrawWins(img, imgSpaceManager.wins);


	cv::waitKey(0);
	cv::destroyAllWindows();

	return 0;
}