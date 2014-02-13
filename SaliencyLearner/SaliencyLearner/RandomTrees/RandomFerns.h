//////////////////////////////////////////////////////////////////////////
// random ferns
// jie feng 2013-08-02
//////////////////////////////////////////////////////////////////////////

#pragma once

#include "common_libs.h"
#include "DecisionTree.h"

namespace learners
{
	/*
		fern is a set of random test for samples
		each test result is mapped to a binary value (left node or right node)
	*/
  
  struct FernTrainingParams 
	{
		int num_features;

		FernTrainingParams()
		{
				num_features = 10;
		}
	};

	class Fern
	{
	private:

		// leaf statistics;
		// use the same structure but each bin represent a test code instead of a class
		std::vector<NodeStatisticsHist> class_likelihood;
		// tests
		std::vector<PixelPairComparisonFeature> features;

		bool m_ifTrained;

	public:
		Fern(): m_ifTrained(false)
		{}

		// training methods
		bool Train(const cv::Mat& samps, const cv::Mat& labels, const FernTrainingParams& params = FernTrainingParams());
		bool Train(const vector<cv::Mat>& samp_imgs, const cv::Mat& labels, const FernTrainingParams& params = FernTrainingParams());

		// predict methods
		int Predict(const cv::Mat& samp);
		int PredictImage(const cv::Mat& samp_img, vector<double>& all_scores);

	};

	////////////////////////////////////////////////////////////////

	struct RandomFernsTrainingParams 
	{
		int num_ferns;
		FernTrainingParams fernParams;

		RandomFernsTrainingParams()
		{
				num_ferns = 20;
		}
	};

	class RandomFerns
	{
	private:
		vector<Fern> ferns;

		bool m_ifTrained;

	public:
			RandomFerns(): m_ifTrained(false) {}

		// training methods
		bool Train(const cv::Mat& samps, const cv::Mat& labels, const RandomFernsTrainingParams& params = RandomFernsTrainingParams());
		bool Train(const vector<cv::Mat>& samp_imgs, const cv::Mat& labels, const RandomFernsTrainingParams& params = RandomFernsTrainingParams());

		// predict methods
		int Predict(const cv::Mat& samp);
		int PredictImage(const cv::Mat& samp_img, vector<double>& all_scores);

	};
}


