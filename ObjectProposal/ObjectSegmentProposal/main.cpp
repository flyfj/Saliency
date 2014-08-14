// ObjectSegmentProposal.cpp : Defines the entry point for the console application.
//

#include "IterativeSegmentor.h"
#include "DataManager/NYUDepth2DataMan.h"

int main()
{
	NYUDepth2DataMan nyuman;
	FileInfos files;
	nyuman.GetImageList(files);
	Mat img = imread(files[0].filepath);
	resize(img, img, Size(300,300));
	objectproposal::IterativeSegmentor iterSegmentor;
	iterSegmentor.verbose = true;
	iterSegmentor.Init();
	iterSegmentor.Run(img);
	
	waitKey(0);
	return 0;
}

