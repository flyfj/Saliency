//////////////////////////////////////////////////////////////////////////
// self implemented decision trees
// jiefeng@2013-4-28
//////////////////////////////////////////////////////////////////////////

#pragma once

#include <opencv2/opencv.hpp>
//#include "common_libs.h"
#include <map>
#include <vector>
#include <time.h>
#include <list>
#include <queue>
#include <fstream>
#include <iostream>
using namespace std;


#define PatchAsSample

namespace learners
{
		enum OptimalCriterion
		{
			SEP_CRIT_IG,
			SEP_CRIT_NEG_REJ
		};

		//////////////////////////////////////////////////////////////////////////
	class NodeStatisticsHist
	{
	public:

		// sample count
		vector<double> bins;
		double sampleCount;

		// sample weights
		vector<double> class_weights;
		double weight_sum;

	public:
		double Entropy() const;

		NodeStatisticsHist();
		NodeStatisticsHist(int nClasses);
		
		// regular probability access
		double GetProbability(int classIndex) const;
		int GetMaxBinId() const;
		double GetMaxProbability() const;
		std::vector<double> GetAllProbability() const;

		// weighted probability access
		int GetMaxWeightedBinId() const;
		double GetWeightedProbability(int classIndex) const;
		double GetMaxWeightedProbability() const;
		std::vector<double> GetAllWeightedProbability() const;
		
		int ClassNum() const { return bins.size(); }

		int SampleNum() const { return (int)sampleCount; }

		// add sample to statistics
		void AddSamples(const cv::Mat& labels, const std::vector<int>& label_ids, std::vector<double>& weights = std::vector<double>());
		void AddSample(int id, int num, double weight = 1) 
		{ 
				sampleCount += num; 
				bins[id] += num; 
				class_weights[id] += weight*num;
				weight_sum += weight*num;
		}

		static double NodeStatisticsHist::EvaluateSeparability(const NodeStatisticsHist& parent, 
				const NodeStatisticsHist& leftChild, const NodeStatisticsHist& rightChild, OptimalCriterion opt_crit);

	private:
		static double ComputeInformationGain(const NodeStatisticsHist& parent, 
			const NodeStatisticsHist& leftChild, const NodeStatisticsHist& rightChild);

		// assume 0 is negative, 1 is positive
		static double ComputeNegativeRejection(const NodeStatisticsHist& parent, 
				const NodeStatisticsHist& leftChild, const NodeStatisticsHist& rightChild);
		
	};

	//////////////////////////////////////////////////////////////////////////

	enum DTreeFeature
	{
			DTREE_FEAT_AXIS,
			DTREE_FEAT_PAIR
	};

	class INodeFeature
	{
	public:
			virtual double GetFeatureResponse(const cv::Mat& feat) const = 0;
	};


	// axis aligned feature
	class AxisFeature
	{
	private:
		int axisIdx;	// index of selected feature dimension

	public:
		AxisFeature()
		{
			axisIdx = 0;
		}

		AxisFeature(int id)
		{
			axisIdx = id;
		}

		int GetAxisId() const
		{
				return axisIdx;
		}

		void SetAxisId(int id)
		{
				if(id < 0)
						return;

				axisIdx = id;
		}

		double GetFeatureResponse(const cv::Mat& feat) const
		{
			assert(feat.cols > axisIdx);

			return feat.at<float>(axisIdx);
		}

	};


	// cross-scale pixel comparison
	struct PixelPair
	{
		cv::Point2f pair_loc1;		// 0 ~ 1, relative to window
		cv::Point2f pair_loc2;
	};

	class PixelPairComparisonFeature
	{
	private:
			PixelPair pixel_pair;

	public:

			PixelPairComparisonFeature()
			{
					// generate comparison pair randomly
					pixel_pair.pair_loc1.x = (double)rand() / (double)RAND_MAX;
					pixel_pair.pair_loc1.y = (double)rand() / (double)RAND_MAX;
					pixel_pair.pair_loc2.x = (double)rand() / (double)RAND_MAX;
					pixel_pair.pair_loc2.y = (double)rand() / (double)RAND_MAX;
			}

			PixelPairComparisonFeature(cv::Point2f loc1, cv::Point2f loc2)
			{
					pixel_pair.pair_loc1 = loc1;
					pixel_pair.pair_loc2 = loc2;
			}

			PixelPair GetFeature() { return pixel_pair; }

			friend std::ostream& operator << (std::ostream& out, const PixelPairComparisonFeature& pixel_pair_feat)
			{
					out<<pixel_pair_feat.pixel_pair.pair_loc1.x<<" "<<pixel_pair_feat.pixel_pair.pair_loc1.y<<" "
							<<pixel_pair_feat.pixel_pair.pair_loc2.x<<" "<<pixel_pair_feat.pixel_pair.pair_loc2.y;
					return out;
			}

			friend std::istream& operator >> (std::istream& in, PixelPairComparisonFeature& pixel_pair_feat)
			{
					in>>pixel_pair_feat.pixel_pair.pair_loc1.x>>pixel_pair_feat.pixel_pair.pair_loc1.y
							>>pixel_pair_feat.pixel_pair.pair_loc2.x>>pixel_pair_feat.pixel_pair.pair_loc2.y;
					return in;
			}

			static std::vector<PixelPairComparisonFeature> GenAllFeatures(int num_bins);
			static std::vector<PixelPairComparisonFeature> GenAllCenterFeatures(int num_bins);

			inline double GetFeatureResponse(const cv::Mat& img)
			{
					//assert(feat.depth() == CV_32F);

					cv::Point pt1, pt2;
					pt1.x = (int)(img.cols * pixel_pair.pair_loc1.x);
					pt1.y = (int)(img.rows * pixel_pair.pair_loc1.y);
					pt2.x = (int)(img.cols * pixel_pair.pair_loc2.x);
					pt2.y = (int)(img.rows * pixel_pair.pair_loc2.y);

					return (double)abs(img.at<uchar>(pt1) - img.at<uchar>(pt2));
			}
	};


	//////////////////////////////////////////////////////////////////////////
	class DecisionTreeNode
	{
	private:

	public:
		int nodeId;
		int parentNodeId;
		bool isLeaf;
		bool isSplit;
		int nodeLevel;	// start from 0

		std::vector<int> samp_ids;	// contained sample ids

		AxisFeature axis_feature;	// general feature
		PixelPairComparisonFeature pair_feature;

		double th;	// threshold
		NodeStatisticsHist trainingDataStatistics;

		double quality_measure;

		DecisionTreeNode();
		DecisionTreeNode(int nClass);
	};


	//////////////////////////////////////////////////////////////////////////

	struct DTreeTrainingParams 
	{
		int MaxLevel;
		double IG_Diff;			// difference between information gain
		double STOP_DIFF;		// general stopping value difference
		double STOP_CONF;		// stopping confidence
		int feature_num;		// test feature number
		int th_num;					// threshold number
		int min_samp_num;		// per node (leaf)
		int fastLevel;
		double conf_th;			// how much to think as confident

		std::vector<int> fast_feats_dims;
		std::map<int, std::string> fast_feats_names;

		DTreeFeature feat_type;
		OptimalCriterion opt_crit;

		DTreeTrainingParams()
		{
			MaxLevel = 10;
			fastLevel = 5;
			min_samp_num = 10;
			IG_Diff = 0;
			STOP_DIFF = 0.00001f;
			STOP_CONF = 0.9f;
			feature_num = 300;
			th_num = 30;
			feat_type = DTREE_FEAT_PAIR;
			conf_th = 0.7f;
			opt_crit = SEP_CRIT_IG;

			fast_feats_dims.push_back(3);	// h
			fast_feats_names[3] = "h";
			fast_feats_dims.push_back(4);	// s
			fast_feats_names[4] = "s";
			fast_feats_dims.push_back(5);	// v
			fast_feats_names[5] = "v";
			fast_feats_dims.push_back(6);	// y
			fast_feats_names[6] = "y";
		}
	};


	////////////////////////////////////////////////////////////////////////////
	class DecisionTree
	{
	private:
		
		bool useFastFeature;
		DTreeTrainingParams defaultParams;
		

	public:
		
		bool m_ifTrained;
		
		std::map<int, DecisionTreeNode> nodes;

		static double easy_sample_ratio;
		static bool ifEasy;
		
		DTreeFeature tree_feat_type;
		int max_tree_level;

		DecisionTree(void);

		const std::map<int, DecisionTreeNode>& GetNodes() const
		{
				return nodes;
		}

		// general Mat row format sample and labels
		// labels must be continuous integers starting from 0
		bool TrainTree(const cv::Mat& samps, const cv::Mat& labels, 
				const DTreeTrainingParams& train_params, std::vector<double>& samp_weights = std::vector<double>());

		int Predict(const cv::Mat& samp, std::vector<double>& all_scores);

		//////////////////////////////////////////////////////////////////////////
		// patch learning using pixel pair comparison
		bool TrainTree(const std::vector<cv::Mat>& samp_imgs, const cv::Mat& labels, const DTreeTrainingParams& train_params);

		int PredictImage(const cv::Mat& img, std::vector<double>& all_scores);

		//////////////////////////////////////////////////////////////////////////
		bool EvaluateDecisionTree(const cv::Mat& test_samps, const cv::Mat& test_labels, int nClass);

		void DrawPixelDecisionBoundary(int width, int height, int id);

		void DrawPixelWeightMap(const cv::Mat& samps, const std::vector<double>& weights, int id);

		//////////////////////////////////////////////////////////////////////////

		bool Save(const std::string savefile);

		bool Load(const std::string loadfile);
	};
}



