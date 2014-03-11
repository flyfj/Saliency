

#include "ImageSpaceManager.h"
#include "WindowEvaluator.h"
#include <string>
using namespace std;

int main()
{
	std::string imgdir = "E:\\Datasets\\VOC2007\\VOCtrainval_06-Nov-2007\\VOC2007\\JPEGImages\\";
	std::string imgname = "000012";

	// load image
	cv::Mat img = cv::imread(imgdir + imgname + ".jpg");
	cv::imshow("input", img);
	cv::waitKey(10);

	// load ground truth
	map<string, vector<ImgWin>> gt_wins;
	WindowEvaluator eval;
	eval.gtdir = "E:\\Datasets\\VOC2007\\VOC2007_AnnotationsOpenCV_Readable\\";
	vector<string> imgnames;
	imgnames.push_back(imgname);
	eval.LoadVOC07Boxes(imgnames, gt_wins);

	
	Tools::DrawWins(img, gt_wins[imgname]);
	cv::imshow("gt", img);
	cv::waitKey(0);

	// process
	double start_t = cv::getTickCount();
	
	ImageSpaceManager imgSpaceManager;
	imgSpaceManager.DivideImage(img);

	std::cout<<"Process time: "<<(cv::getTickCount()-start_t) / cv::getTickFrequency()<<"s."<<std::endl;

	Tools::DrawWins(img, imgSpaceManager.wins);


	cv::waitKey(0);
	cv::destroyAllWindows();

	return 0;
}