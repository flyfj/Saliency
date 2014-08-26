// ObjectSegmentProposal.cpp : Defines the entry point for the console application.
//

#include "ObjSegmentProposal.h"
#include "IO/Dataset/NYUDepth2DataMan.h"
#include "ObjectRanker.h"
#include "ObjProposalDemo.h"

int main()
{
	ObjProposalDemo demo;
	Mat cimg = imread("E:\\res\\objectness\\51_1.jpg");
	demo.RunObjWinProposal(cimg, Mat());
	//demo.RunObjWinProposal();
	return 0;

	visualsearch::processors::attention::ObjectRanker ranker;
	ranker.LearnObjectWindowPredictor();
	getchar();
	return 0;

	// get input
	visualsearch::io::dataset::NYUDepth2DataMan nyuman;
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

