#pragma once



#include "RandomTrees/DecisionTree.h"
#include "ImageSegmentor.h"
#include "ColorDescriptors.h"
#include "Tools.h"

using namespace learners;
using namespace tools;
using namespace visualsearch;
using namespace visualsearch::features;


class SaliencyLearner
{
private:

	learners::DecisionTree dtree;
	ImageSegmentor segmentor;
	ColorDescriptors colorfeat;

	cv::Mat spSimMat;

	cv::Size newImgSize;

	ObjectCategory trainSamples;

public:

	int spNum;
	vector<SuperPixel> sps;

	SaliencyLearner(void);

	bool ComputeSelfSimMap(int idx, cv::Mat& feat, cv::Mat& outmap);

	//////////////////////////////////////////////////////////////////////////
	// data set processing
	//////////////////////////////////////////////////////////////////////////

	bool LoadTrainingData(string imgfoler, string maskfolder);


	//////////////////////////////////////////////////////////////////////////

	// preprocessing
	bool Init(const cv::Mat& colorImg);

	// do preprocessing for input image
	bool Preprocess(const cv::Mat& colorImg);

	bool Train(string savefile, bool ifLoad);

	bool Run(const cv::Mat& colorImg, cv::Mat& salmap, bool verbose = false);

};

