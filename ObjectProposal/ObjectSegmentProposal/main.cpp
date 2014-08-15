// ObjectSegmentProposal.cpp : Defines the entry point for the console application.
//

#include "IterativeSegmentor.h"
#include "DataManager/NYUDepth2DataMan.h"

int main()
{
	NYUDepth2DataMan nyuman;
	FileInfos imgfiles, dmapfiles;
	nyuman.GetImageList(imgfiles);
	nyuman.GetDepthmapList(dmapfiles);
	Mat img = imread(imgfiles[5].filepath);
	Mat dmap;
	nyuman.LoadDepthData(dmapfiles[5].filepath, dmap);
	resize(img, img, Size(300,300));
	resize(dmap, dmap, Size(300,300));
	normalize(dmap, dmap, 1, 0, NORM_MINMAX);
	objectproposal::IterativeSegmentor iterSegmentor;
	iterSegmentor.verbose = true;
	iterSegmentor.Init();
	iterSegmentor.Run(img, dmap);
	
	waitKey(0);
	return 0;
}

