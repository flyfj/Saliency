// ObjectSegmentProposal.cpp : Defines the entry point for the console application.
//

#include "ObjSegmentProposal.h"
#include "IO/Dataset/NYUDepth2DataMan.h"
#include "ObjectRanker.h"
#include "ObjProposalDemo.h"
#include "Common/Tools/RGBDTools.h"
#include "Processors/Segmentation/IterativeSegmentor.h"
#include "Tester.h"

// link
// libs
#ifdef _DEBUG
#pragma comment(lib, "opencv_core249d.lib")
#pragma comment(lib, "opencv_imgproc249d.lib")
#pragma comment(lib, "opencv_highgui249d.lib")
#pragma comment(lib, "opencv_ml249d.lib")
#pragma comment(lib, "opencv_features2d249d.lib")
#pragma comment(lib, "opencv_nonfree249d.lib")
#pragma comment(lib, "opencv_objdetect249d.lib")
#pragma comment(lib, "opencv_flann249d.lib")
#else
#pragma comment(lib, "opencv_core249.lib")
#pragma comment(lib, "opencv_imgproc249.lib")
#pragma comment(lib, "opencv_highgui249.lib")
#pragma comment(lib, "opencv_ml249.lib")
#pragma comment(lib, "opencv_features2d249.lib")
#pragma comment(lib, "opencv_nonfree249.lib")
#pragma comment(lib, "opencv_objdetect249.lib")
#pragma comment(lib, "opencv_flann249.lib")
#endif



int main()
{
	/*ObjectProposalTester tester;
	tester.TestRankerLearner();
	getchar();
	return 0;*/

	Mat dmap, cimg;
	visualsearch::common::tools::RGBDTools rgbdtool;
	objectproposal::ObjSegmentProposal prop;
	visualsearch::processors::segmentation::IterativeSegmentor segmentor;
	ObjProposalDemo demo;
	cimg = imread("E:\\Datasets\\RGBD_Dataset\\Saliency\\RGB\\1_03-14-28.jpg");
	dmap = imread("E:\\Datasets\\RGBD_Dataset\\Saliency\\Depth\\smoothedDepth\\1_03-14-28_Depth.png");
	dmap.convertTo(dmap, CV_32F);
	demo.RunObjSegProposal(cimg, dmap);
	waitKey(0);
	//demo.RunVideoDemo(SENSOR_KINECT, DEMO_OBJECT_SEG);

	return 0;
	
	/*visualsearch::common::tools::RGBDTools rgbdtool;
	dmap = imread("D:\\imgs\\depth.png", CV_LOAD_IMAGE_UNCHANGED);
	dmap.convertTo(dmap, CV_32F);
	Mat pts;
	rgbdtool.KinectDepthTo3D(dmap, pts);
	rgbdtool.SavePointsToOBJ("d:\\test.obj", pts);
	return 0;*/


	segmentor.PrepareMergerTrainingSamples();
	getchar();
	return 0;

	
	cimg = imread("d:\\imgs\\img_0263.png");
	dmap = imread("D:\\imgs\\depth.png", CV_LOAD_IMAGE_UNCHANGED);
	dmap.convertTo(dmap, CV_32F);
	Mat pts;
	vector<SuperPixel> sps;
	segmentor.verbose = true;
	segmentor.Init(cimg, dmap);
	segmentor.Run();
	segmentor.SaveResults("E:\\res\\seg_prop\\", cimg, sps);
	//rgbdtool.KinectDepthTo3D(dmap, pts);
	//prop.Compute3DDistMap(dmap, Mat());
	//rgbdtool.SavePointsToOBJ("d:\\test.obj", pts);
	//return 0;

	demo.RunVideoDemo(SENSOR_KINECT, DEMO_VIEW_ONLY);
	return 0;
	//demo.RunVideoDemo(SENSOR_KINECT, DEMO_OBJECT_SEG);
	
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

