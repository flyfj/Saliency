//////////////////////////////////////////////////////////////////////////
// ranker for candiate object segments or windows
// jiefeng@2014-08-12
//////////////////////////////////////////////////////////////////////////


#pragma once

#include "Common/common_libs.h"
#include "Processors/Segmentation/ImageSegmentor.h"
#include "Common/tools/Tools.h"
#include "Features/Color/ColorDescriptors.h"
#include "Features/DepthDescriptors.h"
#include "IO/Dataset/NYUDepth2DataMan.h"
#include "Processors/Segmentation/SegmentProcessor.h"
#include "Processors/Attention/SaliencyComputer.h"
#include "Processors/ShapeAnalyzer.h"

namespace visualsearch
{
	using namespace features;
	using namespace io::dataset;

	namespace processors
	{
		using namespace segmentation;

		namespace attention
		{
			enum SegmentRankType
			{
				SEG_RANK_CC,
				SEG_RANK_SALIENCY
			};

			// use saliency or prior to rank object segment
			class ObjectRanker
			{
			public:
				ObjectRanker(void);

				bool RankWindowsBySaliency(const Mat& cimg, vector<ImgWin>& wins, vector<int>& ordered_win_ids);

				// classifier if a superpixel is an object based a set of features
				bool LearnObjectPredictorFromNYUDepth();
				bool LearnObjectWindowPredictor();

				bool RankSegments(const Mat& cimg, const Mat& dmap, const vector<SuperPixel>& sps, SegmentRankType rtype, vector<int>& orded_sp_ids);

				bool RankWindows(const Mat& cimg, const Mat& dmap, const vector<ImgWin>& wins, vector<int>& orded_win_ids);

			private:
				float ComputeCenterSurroundColorContrast(const Mat& cimg, const SuperPixel& sp);

				bool RankSegmentsBySaliency(const Mat& cimg, const vector<SuperPixel>& sps, vector<int>& orded_sp_ids);

				bool RankSegmentsByCC(const Mat& cimg, const vector<SuperPixel>& sps, vector<int>& orded_sp_ids);
				
				//////////////////////////////////////////////////////////////////////////

				bool ComputeSegmentRankFeature(const Mat& cimg, const Mat& dmap, SuperPixel& sp, Mat& feat);

				bool ComputeWindowRankFeatures(const Mat& cimg, const Mat& dmap, vector<ImgWin>& wins, vector<Mat>& feats);

				processors::segmentation::SegmentProcessor segprocessor;
				features::color::ColorDescriptors colordesc;
				SaliencyComputer salcomputer;
			};
		}
	}

	
}


