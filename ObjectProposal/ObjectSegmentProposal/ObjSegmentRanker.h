//////////////////////////////////////////////////////////////////////////
// ranker for candiate object segments
// jiefeng@2014-08-12
//////////////////////////////////////////////////////////////////////////


#pragma once

#include "Common/common_libs.h"
#include "Processors/ImageSegmentor.h"
#include "Common/Tools.h"
#include "Features/ColorDescriptors.h"
#include "Features/DepthDescriptors.h"
#include "DataManager/NYUDepth2DataMan.h"
#include "SegmentProcessor.h"

namespace objectproposal
{
	using namespace visualsearch;

	enum SegmentRankType
	{
		SEG_RANK_CC
	};

	// use saliency or prior to rank object segment
	class ObjSegmentRanker
	{
	private:
		float ComputeCenterSurroundColorContrast(const Mat& cimg, const SuperPixel& sp);
		bool RankSegmentsByCC(const Mat& cimg, const vector<SuperPixel>& sps, vector<int>& orded_sp_ids);
		bool ComputeSegmentRankFeature(const Mat& cimg, const Mat& dmap, SuperPixel& sp, Mat& feat);
		
		SegmentProcessor segprocessor;
		features::ColorDescriptors colordesc;

	public:
		ObjSegmentRanker(void);

		// classifier if a superpixel is an object based a set of features
		bool LearnObjectPredictorFromNYUDepth();

		bool RankSegments(const Mat& cimg, const Mat& dmap, const vector<SuperPixel>& sps, SegmentRankType rtype, vector<int>& orded_sp_ids);
		
	};
}


