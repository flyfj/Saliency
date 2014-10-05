//////////////////////////////////////////////////////////////////////////
// ranker for candidate object segments or windows
// jiefeng@2014-08-12
//////////////////////////////////////////////////////////////////////////


#pragma once

#include <memory>

#include "Common/common_libs.h"
#include "Common/tools/Tools.h"
#include "Processors/Segmentation/ImageSegmentor.h"
#include "Features/Color/ColorDescriptors.h"
#include "Features/DepthDescriptors.h"
#include "IO/Dataset/NYUDepth2DataMan.h"
#include "IO/Dataset/RGBDECCV14.hpp"
#include "Processors/Segmentation/SegmentProcessor.h"
#include "Processors/Attention/SaliencyComputer.h"
#include "Processors/ShapeAnalyzer.h"
#include "Learners/LearnerTools.hpp"
#include "Learners/RandomTrees/RandomForest.h"


namespace visualsearch {
	namespace processors {
		namespace attention {
			using namespace features;
			using namespace io::dataset;
			using namespace segmentation;

			enum SegmentRankType
			{
				SEG_RANK_CC,
				SEG_RANK_SALIENCY,
				SEG_RANK_LEARN
			};

			// rank segment based on objectness
			class ObjectRanker
			{
			public:
				ObjectRanker(void);

				// classifier if a superpixel is an object based a set of features
				bool LearnObjectPredictor();
				bool LearnObjectWindowPredictor();

				bool RankWindowsBySaliency(const Mat& cimg, vector<ImgWin>& wins, vector<int>& ordered_win_ids);

				bool RankSegments(const Mat& cimg, const Mat& dmap, const vector<SuperPixel>& sps, SegmentRankType rtype, vector<int>& orded_sp_ids);

				bool RankWindows(const Mat& cimg, const Mat& dmap, const vector<ImgWin>& wins, vector<int>& orded_win_ids);

			private:
				// hand-craft rank methods
				float ComputeCenterSurroundColorContrast(const Mat& cimg, const SuperPixel& sp);

				bool RankSegmentsBySaliency(const Mat& cimg, const Mat& dmap, const vector<SuperPixel>& sps, vector<int>& orded_sp_ids);

				bool RankSegmentsByCC(const Mat& cimg, const vector<SuperPixel>& sps, vector<int>& orded_sp_ids);

				//////////////////////////////////////////////////////////////////////////

				bool ComputeSegmentRankFeature(const Mat& cimg, const Mat& dmap, SuperPixel& sp, Mat& feat);

				bool ComputeWindowRankFeatures(const Mat& cimg, const Mat& dmap, vector<ImgWin>& wins, vector<Mat>& feats);

				//////////////////////////////////////////////////////////////////////////
				bool PrepareRankTrainData(DatasetName dbs);

				//////////////////////////////////////////////////////////////////////////
				processors::segmentation::SegmentProcessor segprocessor;
				features::color::ColorDescriptors colordesc;
				SaliencyComputer salcomputer;

				Mat rank_train_data, rank_train_label;
				Mat rank_test_data, rank_test_label;
			};
		}
	}

	
}


