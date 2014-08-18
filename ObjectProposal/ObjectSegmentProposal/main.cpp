// ObjectSegmentProposal.cpp : Defines the entry point for the console application.
//

#include "ObjSegmentProposal.h"
#include "DataManager/NYUDepth2DataMan.h"
#include "ObjectRanker.h"

int main()
{
	visualsearch::processors::attention::ObjectRanker ranker;
	ranker.LearnObjectWindowPredictor();
	getchar();
	return 0;

	// get input
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

	

	// process
	objectproposal::ObjSegmentProposal processor;
	vector<SuperPixel> res;
	processor.Run(img, dmap, 10, res);
	processor.VisProposals(img, res);
	
	waitKey(0);
	return 0;
}

