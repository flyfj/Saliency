// ObjectSegmentProposal.cpp : Defines the entry point for the console application.
//

#include "IterativeSegmentor.h"

int main()
{
	Mat img = imread("D:\\imgs\\test.jpg");
	objectproposal::IterativeSegmentor iterSegmentor;
	iterSegmentor.Init();
	iterSegmentor.Run(img);
	
	return 0;
}

