//////////////////////////////////////////////////////////////////////////
// self implemented decision trees
// jiefeng@2013-4-28
//////////////////////////////////////////////////////////////////////////

#pragma once

#include <opencv2/opencv.hpp>
//#include "common_libs.h"
#include <map>
#include <vector>
#include <list>
#include <queue>
#include <fstream>
#include <iostream>
using namespace std;
using namespace cv;
//using namespace visualsearch;

namespace learners
{
	class NodeStatisticsHist
	{
	public:
		vector<float> bins;	// original count
		double sampleCount;

	public:
		double Entropy() const;

		NodeStatisticsHist();
		NodeStatisticsHist(int nClasses);

		float GetProbability(int classIndex) const;
		int GetMaxBinId() const;
		float GetMaxProbability() const;
		vector<float> GetAllProbability() const;
		int ClassNum() const { return bins.size(); }
		int SampleNum() const { return sampleCount; }
		// add sample to statistics
		void AddSamples(const Mat& labels, const vector<int>& label_ids);
		void AddSample(int id) { sampleCount++; bins[id]++; }

		static double ComputeInformationGain(const NodeStatisticsHist& parent, 
			const NodeStatisticsHist& leftChild, const NodeStatisticsHist& rightChild);
	};


	class AxisFeature
	{
	public:
		int axisIdx;	// index of selected feature dimension

		AxisFeature()
		{
			axisIdx = 0;
		}

		AxisFeature(int id)
		{
			axisIdx = id;
		}

		float GetFeatureResponse(const Mat& feat)
		{
			assert(feat.cols > axisIdx);

			return feat.at<float>(axisIdx);
		}


	};

	class DecisionTreeNode
	{
	private:

	public:
		int nodeId;
		bool isLeaf;
		bool isSplit;
		int nodeLevel;

		vector<int> samp_ids;	// containing samples

		AxisFeature feature;
		float th;	// threshold
		NodeStatisticsHist trainingDataStatistics;

		float clf_confidence;

		DecisionTreeNode();
		DecisionTreeNode(int nClass);

	};

	struct DTreeTrainingParams 
	{
		int MaxLevel;
		float IG_Diff;	// difference between information gain
		int feature_num;
		int th_num;
		int fastLevel;
		float conf_th;	// how much to think as confident

		vector<int> fast_feats_dims;
		map<int, string> fast_feats_names;


		DTreeTrainingParams()
		{
			MaxLevel = 10;
			fastLevel = 5;
			IG_Diff = 0.001f;
			feature_num = 300;
			th_num = 20;
			conf_th = 0.7f;
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

	class DecisionTree
	{
	private:
		map<int, DecisionTreeNode> nodes;

		bool useFastFeature;

		DTreeTrainingParams defaultParams;

	public:

		static float easy_sample_ratio;
		static bool ifEasy;

		DecisionTree(void);

		bool TrainTree(const Mat& samps, const Mat& labels, const DTreeTrainingParams& train_params);

		int Predict(const Mat& samp, vector<float>& all_scores);

		bool Save(const string savefile);

		bool Load(const string loadfile);
	};
}



