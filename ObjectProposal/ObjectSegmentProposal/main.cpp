// ObjectSegmentProposal.cpp : Defines the entry point for the console application.
//

#include "ObjSegmentProposal.h"
#include "IO/Dataset/NYUDepth2DataMan.h"
#include "ObjectRanker.h"
#include "ObjProposalDemo.h"
#include "RGBDTools.h"

int main()
{
	Mat dmap, cimg;
	/*visualsearch::common::tools::RGBDTools rgbdtool;
	dmap = imread("D:\\imgs\\depth.png", CV_LOAD_IMAGE_UNCHANGED);
	dmap.convertTo(dmap, CV_32F);
	Mat pts;
	rgbdtool.KinectDepthTo3D(dmap, pts);
	rgbdtool.SavePointsToOBJ("d:\\test.obj", pts);
	return 0;*/

	ObjProposalDemo demo;
	demo.RunVideoDemo(SENSOR_KINECT, DEMO_VIEW_ONLY);
	return 0;
	//demo.RunVideoDemo(SENSOR_KINECT, DEMO_OBJECT_SEG);
	cimg = imread("d:\\imgs\\img_0263.png");
	demo.RunObjSegProposal(cimg, Mat());
	waitKey(0);
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

