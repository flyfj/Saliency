//////////////////////////////////////////////////////////////////////////
// ranker for candiate object segments or windows
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
#include "Processors/Saliency/SaliencyComputer.h"
#include "Processors/ShapeAnalyzer.h"

namespace visualsearch
{
	namespace processors
	{
		namespace attention
		{
			enum SegmentRankType
			{
				SEG_RANK_CC
			};

			// use saliency or prior to rank object segment
			class ObjectRanker
			{
			private:
				float ComputeCenterSurroundColorContrast(const Mat& cimg, const SuperPixel& sp);
				
				bool RankSegmentsByCC(const Mat& cimg, const vector<SuperPixel>& sps, vector<int>& orded_sp_ids);

				bool ComputeSegmentRankFeature(const Mat& cimg, const Mat& dmap, SuperPixel& sp, Mat& feat);

				bool ComputeWindowRankFeatures(const Mat& cimg, const Mat& dmap, const vector<ImgWin>& wins, vector<Mat>& feats);

				objectproposal::SegmentProcessor segprocessor;
				features::ColorDescriptors colordesc;
				SaliencyComputer salcomputer;

			public:
				ObjectRanker(void);

				bool RankWindowsBySaliency(const Mat& cimg, const vector<ImgWin>& wins, vector<int>& ordered_win_ids);

				// classifier if a superpixel is an object based a set of features
				bool LearnObjectPredictorFromNYUDepth();
				bool LearnObjectWindowPredictor();

				bool RankSegments(const Mat& cimg, const Mat& dmap, const vector<SuperPixel>& sps, SegmentRankType rtype, vector<int>& orded_sp_ids);

				bool RankWindows(const Mat& cimg, const Mat& dmap, const vector<ImgWin>& wins, vector<int>& orded_win_ids);

			};
		}
	}

	
}


