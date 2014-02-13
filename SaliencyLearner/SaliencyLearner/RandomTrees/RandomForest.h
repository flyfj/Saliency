//////////////////////////////////////////////////////////////////////////
// self implemented random forest
// jiefeng@2013-7-23
//////////////////////////////////////////////////////////////////////////


#pragma once

#include "DecisionTree.h"
#include "Tools.h"
#include <functional>

namespace learners
{

		struct RForestTrainingParams
		{
				int num_trees;
				// if disjoint, train samples for each tree are picked with 
				// no overlap by dividing training set evenly;
				// otherwise, randomly pick percentage samples
				bool split_disjoint;

				DTreeTrainingParams tree_params;

				RForestTrainingParams()
				{
						num_trees = 10;
						split_disjoint = true;
				}
		};

		class RandomForest
		{
		private:

				std::vector<learners::DecisionTree> tree_collection;
				int nClass;

		public:

				bool m_ifTrained;

				RandomForest(void) { m_ifTrained = false; nClass = 1; }
				~RandomForest(void) {}

				bool Train(const cv::Mat& samps, const cv::Mat& labels, const RForestTrainingParams& params);
				bool Train(const std::vector<cv::Mat>& samp_imgs, const cv::Mat& labels, const RForestTrainingParams& params);

				int ForestPredict(const cv::Mat& samp, std::vector<double>& all_scores);
				int PredictImage(const cv::Mat& img, std::vector<double>& all_scores);

				bool Save(const std::string savefile);
				bool Load(const std::string loadfile);

				bool EvaluateRandomForest(const cv::Mat& test_samps, const cv::Mat& test_labels, int nClass);

				void DrawPixelDecisionBoundary(int width, int height, int id);
				void DrawPixelWeightMap(const cv::Mat& samps, const std::vector<double>& weights, int id);

		};
}


