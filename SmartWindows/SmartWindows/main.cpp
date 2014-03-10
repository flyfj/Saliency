

#include "ImageSpaceManager.h"


int main()
{
	cv::Mat img = cv::imread("F:\\Images\\2_75_75971.jpg");
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