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
