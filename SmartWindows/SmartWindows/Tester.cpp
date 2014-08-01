#include "Tester.h"


Tester::Tester(void)
{
}


void Tester::TestFixationSegmentation()
{
	visualsearch::FixationSegmentor fixsegmentor;
	string dmapfn = "E:\\Datasets\\RGBD_Dataset\\Berkeley\\VOCB3DO\\RegisteredDepthData\\img_0016_abs_smooth.png";
	string cimgfn = "E:\\Datasets\\RGBD_Dataset\\Berkeley\\VOCB3DO\\KinectColor\\img_0016.png";
	Mat cimg = imread(cimgfn);
	Mat dmap = imread(dmapfn);
	resize(cimg, cimg, Size(300, 300));
	resize(dmap, dmap, Size(300, 300));
	fixsegmentor.DoSegmentation(Point(150, 150), cimg, dmap, Mat());
}

void Tester::TestViewSearch()
{
	Mat queryimg = imread("‪E:\\1_25_25406.jpg");

	visualsearch::ViewSearcher vsearcher;
	vsearcher.LoadCategoryDepthMaps("E:\\Datasets\\RGBD_Dataset\\UW\\rgbd-dataset\\");

	vsearcher.LearnOptimalBinaryCodes(32);
	vsearcher.BuildHashTable();

	//Mat queryimg = imread("E:\\Images\\1_26_26745.jpg");
	queryimg.convertTo(queryimg, CV_32F);
	visualsearch::ImgVisualizer::DrawFloatImg("query", queryimg, Mat());
	waitKey(0);
	vsearcher.Search(queryimg, vector<int>());
	waitKey(0);
}
