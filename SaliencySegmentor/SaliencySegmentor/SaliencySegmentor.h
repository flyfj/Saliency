//////////////////////////////////////////////////////////////////////////
// perform iterative saliency segmentation
// jiefeng @ 2012.12.11
//////////////////////////////////////////////////////////////////////////


#pragma once


#include "common.h"
#include "ImageSegmentor.h"
#include "SaliencyComputer.h"
#include <iostream>
#include <set>
using namespace std;
using namespace cv;


namespace Saliency
{

	class SaliencySegmentor: public Mat, protected Rect
	{

	private:

		vector<SegSuperPixelFeature> sp_features;
		Mat lab_img;
		int max_id;
		vector<bool> merged_sign;	// sign indicating if each segment has been merged 

		// processor
		ImageSegmentor img_segmentor;
		SaliencyComputer sal_computer;

	public:

		SaliencySegmentor(void);
		~SaliencySegmentor(void);

		void Init(const Mat& img);

		// simply combine two segments and create a new one with updated segment data
		void MergeSegments(
			const SegSuperPixelFeature& in_seg1, const SegSuperPixelFeature& in_seg2, 
			SegSuperPixelFeature& out_seg, bool onlyCombineFeat);

		// do iterative merging to find salient object
		bool MineSalientObjectFromSegment(const Mat& img, int start_seg_id);

	};
}


