//////////////////////////////////////////////////////////////////////////
// ranker for candidate object segments or windows
// jiefeng@2014-08-12
//////////////////////////////////////////////////////////////////////////


#pragma once

#include <memory>

#include "Common/common_libs.h"
#include "Common/tools/Tools.h"
#include "Common/tools/ImgVisualizer.h"
#include "Processors/Segmentation/ImageSegmentor.h"
#include "Features/Color/ColorDescriptors.h"
#include "Features/DepthDescriptors.h"
#include "IO/Dataset/NYUDepth2DataMan.h"
#include "IO/Dataset/RGBDECCV14.h"
#include "Processors/Segmentation/SegmentProcessor.h"
#include "Processors/Segmentation/ImageSegmentor.h"
#include "Processors/Attention/SaliencyComputer.h"
#include "Processors/Attention/Composition/SalientRGBDRegionDetector.h"
#include "Processors/Attention/CenterSurroundFeatureContraster.h"
#include "Processors/ShapeAnalyzer.h"
#include "Learners/LearnerTools.h"
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
				bool PrepareRankTrainData(DatasetName dbs);
				bool LearnObjectPredictor();
				bool LearnObjectWindowPredictor();

				bool RankWindowsBySaliency(const Mat& cimg, vector<ImgWin>& wins, vector<int>& ordered_win_ids);

				bool RankSegments(const Mat& cimg, const Mat& dmap, const vector<SuperPixel>& sps, SegmentRankType rtype, vector<int>& orded_sp_ids);

				bool RankWindows(const Mat& cimg, const Mat& dmap, const vector<ImgWin>& wins, vector<int>& orded_win_ids);

			private:
				// hand-craft rank methods
				bool RankSegmentsBySaliency(const Mat& cimg, const Mat& dmap, const vector<SuperPixel>& sps, vector<int>& orded_sp_ids);

				bool RankSegmentsByCC(const Mat& cimg, const vector<SuperPixel>& sps, vector<int>& orded_sp_ids);

				//////////////////////////////////////////////////////////////////////////

				bool ComputeSegmentRankFeature(const Mat& cimg, const Mat& dmap, SuperPixel& sp, Mat& feat);

				bool ComputeWindowRankFeatures(const Mat& cimg, const Mat& dmap, vector<ImgWin>& wins, vector<Mat>& feats);
				

				//////////////////////////////////////////////////////////////////////////
				processors::segmentation::SegmentProcessor segprocessor;
				features::color::ColorDescriptors colordesc;
				features::DepthDescriptors depth_desc_;
				SaliencyComputer salcomputer;
				SalientRGBDRegionDetector sal_comp_;
				CenterSurroundFeatureContraster cs_contraster;
				common::tools::ImgVisualizer img_vis_;

				Mat rank_train_data, rank_train_label;
				Mat rank_test_data, rank_test_label;
				Mat debug_img_;		// used to save intermediate or debug results
				bool test_;
			};
		}
	}

	
}


