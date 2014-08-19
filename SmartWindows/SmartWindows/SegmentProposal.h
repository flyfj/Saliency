#pragma once
//////////////////////////////////////////////////////////////////////////
// object segment proposal
// jiefeng©2014-08-07
//////////////////////////////////////////////////////////////////////////

#include "common.h"
#include "ImgVisualizer.h"
#include "ImageSegmentor.h"

using namespace visualsearch;

class SegmentProposal
{
private:
	vector<Mat> segments;

public:
	SegmentProposal(void);

	// split a segment into two parts
	bool SplitSegment(const Mat& cimg, const Mat& dmap, const Mat& segmask, Mat& seg1, Mat& seg2);

	bool SegmentDepth(const Mat& dmap);

	bool Test1(const Mat& cimg, const Mat& dmap);
};

