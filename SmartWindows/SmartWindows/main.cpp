

#include "ImageSpaceManager.h"
#include <string>

int main()
{
	std::string imgdir = "E:\\Datasets\\Saliency\\MSRA\\ImageB\\";
	cv::Mat img = cv::imread(imgdir + "2_76_76326.jpg");
	cv::imshow("input", img);
	cv::waitKey(10);

	double start_t = cv::getTickCount();

	ImageSpaceManager imgSpaceManager;
	imgSpaceManager.DivideImage(img);

	std::cout<<"Process time: "<<(cv::getTickCount()-start_t) / cv::getTickFrequency()<<"s."<<std::endl;

	imgSpaceManager.DrawWins(img, imgSpaceManager.wins);


	cv::waitKey(0);
	cv::destroyAllWindows();

	return 0;
}