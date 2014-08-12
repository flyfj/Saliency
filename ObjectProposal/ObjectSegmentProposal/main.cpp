// ObjectSegmentProposal.cpp : Defines the entry point for the console application.
//

#include "IterativeSegmentor.h"

int main()
{
	Mat img = imread("D:\\imgs\\a3.jpg");
	resize(img, img, Size(300,300));
	objectproposal::IterativeSegmentor iterSegmentor;
	iterSegmentor.Init();
	iterSegmentor.Run(img);
	
	waitKey(0);
	return 0;
}

