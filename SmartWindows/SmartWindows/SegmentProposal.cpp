#include "SegmentProposal.h"


SegmentProposal::SegmentProposal(void)
{
}

//////////////////////////////////////////////////////////////////////////

bool SegmentProposal::SegmentDepth(const Mat& dmap)
{
	// normalize dmap
	Mat dmap_norm;
	normalize(dmap, dmap_norm, 1, 0, CV_MINMAX);

	visualsearch::ImgVisualizer::DrawFloatImg("dmap", dmap_norm, Mat());

	// evenly divide into N parts, use those values as threshold
	char str[10];
	int cnt = 0;
	for (float th=0.1; th<1; th+=0.05f)
	{
		Mat dmap_th;
		threshold(dmap_norm, dmap_th, th, 1, CV_THRESH_BINARY);
		sprintf_s(str, "%d", cnt++);
		visualsearch::ImgVisualizer::DrawFloatImg(str, dmap_th, Mat());
		waitKey(0);
	}

	waitKey(0);

	return true;
}


bool SegmentProposal::SplitSegment(const Mat& cimg, const Mat& dmap, const Mat& segmask, Mat& seg1, Mat& seg2)
{


	return true;
}

bool SegmentProposal::Test1(const Mat& cimg, const Mat& dmap)
{
	imshow("cimg", cimg);
	visualsearch::ImageSegmentor imgsegmentor;
	imgsegmentor.m_dThresholdK = 50;
	imgsegmentor.DoSegmentation(cimg);
	imshow("segimg", imgsegmentor.m_segImg);
	vector<SuperPixel>& sps = imgsegmentor.superPixels;
	Mat adjMat;
	imgsegmentor.ComputeAdjacencyMat(sps, adjMat);

	// compute a boundary score for all superpixels
	for (size_t i=0; i<sps.size(); i++)
	{
		sps[i].score = 0;
		for (int c=0; c<adjMat.cols; c++)
		{
			if(adjMat.at<uchar>(i, c) > 0)
			{
				float dist = (sps[i].meancolor.val[0]-sps[c].meancolor.val[0])*(sps[i].meancolor.val[0]-sps[c].meancolor.val[0]) + 
					 (sps[i].meancolor.val[1]-sps[c].meancolor.val[1])*(sps[i].meancolor.val[1]-sps[c].meancolor.val[1]) +
					  (sps[i].meancolor.val[2]-sps[c].meancolor.val[2])*(sps[i].meancolor.val[2]-sps[c].meancolor.val[2]);
				dist = sqrt(dist);
				if(dist > sps[i].score)
					sps[i].score = dist;
			}
		}
	}

	// draw score map
	Mat scoremap(cimg.rows, cimg.cols, CV_32F);
	scoremap.setTo(0);

	for (size_t i=0; i<sps.size(); i++)
	{
		scoremap.setTo(sps[i].score, sps[i].mask);
	}
	ImgVisualizer::DrawFloatImg("scoremap", scoremap, Mat());
	waitKey(0);

	return true;
}