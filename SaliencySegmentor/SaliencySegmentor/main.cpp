

#include "SaliencySegmentor.h"
using namespace Saliency;


int main()
{
	Mat img = imread("D:\\imgs\\photo.jpg");
	imshow("img", img);

	SaliencySegmentor segmentor;
	segmentor.Init(img);
	segmentor.MineSalientObjectFromSegment(img, 10);


	waitKey(0);

	return 0;
}