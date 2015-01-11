//////////////////////////////////////////////////////////////////////////
// ranker for candidate object segments or windows
// jiefeng@2014-08-12
//////////////////////////////////////////////////////////////////////////


#pragma once

#include <memory>

#include "stdafx.h"


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
				SEG_RANK_LEARN,
				SEG_RANK_SHAPE,
				SEG_RANK_DEPTH,
				SEG_RANK_PRIOR
			};

			// rank segment based on objectness
			class ObjectRanker
			{
			public:
				ObjectRanker(void);

				bool Init(const Mat& color_img);

				// classifier if a superpixel is an object based a set of features
				bool LearnObjectPredictor(DatasetName db_name);
				bool LearnObjectWindowPredictor();

				bool RankWindowsBySaliency(const Mat& cimg, vector<ImgWin>& wins, vector<int>& ordered_win_ids);

				bool RankSegments(const Mat& cimg, const Mat& dmap, vector<VisualObject>& sps, SegmentRankType rtype, vector<int>& orded_sp_ids);

				bool RankWindows(const Mat& cimg, const Mat& dmap, const vector<ImgWin>& wins, vector<int>& orded_win_ids);

			private:
				// ranking using learned model
				bool RankSegmentsByLearner(const Mat& cimg, const Mat& dmap, vector<VisualObject>& sps, vector<int>& ordered_sp_ids);
				// hand-craft rank methods
				bool RankSegmentsBySaliency(const Mat& cimg, const Mat& dmap, vector<VisualObject>& sps, vector<int>& orded_sp_ids);

				bool RankSegmentsByCC(const Mat& cimg, const vector<VisualObject>& sps, vector<int>& orded_sp_ids);

				bool RankSegmentsByShape(vector<VisualObject>& sps, vector<int>& ordered_sp_ids);

				bool RankSegmentsByPrior(const Mat& cimg, const Mat& dmap, vector<VisualObject>& sps, IntVector& ordered_sp_ids);

				//////////////////////////////////////////////////////////////////////////

				bool ComputeSegmentRankFeature(const Mat& cimg, const Mat& dmap, VisualObject& sp, Mat& feat);

				bool ComputeWindowRankFeatures(const Mat& cimg, const Mat& dmap, vector<ImgWin>& wins, vector<Mat>& feats);
				
				//////////////////////////////////////////////////////////////////////////
				processors::segmentation::SegmentProcessor segprocessor;
				features::color::ColorDescriptors colordesc;
				features::DepthDescriptors depth_desc_;
				SaliencyComputer salcomputer;
				SalientRGBDRegionDetector sal_comp_;
				CenterSurroundFeatureContraster cs_contraster;
				common::tools::ImgVisualizer img_vis_;
				learners::trees::RandomForest<learners::trees::ConvolutionFeature> rforest;
				learners::trees::RForestTrainingParams rfparams;

				Mat rank_train_data, rank_train_label;
				Mat rank_test_data, rank_test_label;
				Mat debug_img_;				// used to save intermediate or debug results
				vector<Mat> sal_maps;		// saliency maps of different types
				Mat gray_img;
				bool test_;
				string ranker_fn;			// filename for storing learned ranker
			};
		}
	}

	
}


