//////////////////////////////////////////////////////////////////////////
//	superpixel segmentation
//	fengjie@cis.pku
//	2011/9/6
//////////////////////////////////////////////////////////////////////////


#pragma once

#include "common.h"
#include <vector>
#include "GraphBasedSegment/wrapper.h"
using namespace cv;




class ImageSegmentor
{

public:

	//segment-related
	float m_dSmoothSigma;
	float m_dThresholdK;
	int m_dMinArea;
	Mat m_segImg;
	Mat m_idxImg;	// superpixel index

public:
	ImageSegmentor(void);
	~ImageSegmentor(void);

	int DoSegmentation(const Mat& img);
};
