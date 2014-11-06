


#pragma once

#include "Common/common_libs.h"
#include "IO/Dataset/NYUDepth2DataMan.h"
#include "Learners/RandomTrees/RandomForest.hpp"
#include "Processors/Segmentation/SegmentProcessor.h"
using namespace visualsearch::common;
using namespace visualsearch::features;
using namespace visualsearch::io::dataset;
using namespace visualsearch::processors::segmentation;
using namespace visualsearch::learners::trees;

class SuperpixelClf
{
public:
	SuperpixelClf(void);

	// first call init; set up parameters
	bool Init(int sp_feats);

	bool Train(DatasetName db_name);

	bool Predict(const Mat& cimg, const Mat& dmap);

	bool Predict(SuperPixel& sp, const Mat& cimg, const Mat& dmap_raw, vector<double>& scores);

private:
	static float LabelDistributionDist(const vector<float>& label1, const vector<float>& label2);

	RForestTrainingParams rfparams;
	RandomForest<LinearFeature> rf;
	SegmentProcessor seg_processor;
	string rf_model_file;
	string db_info_file;
	int sp_feats_;
	map<int, int> label_map;	// label remapping, raw label -> new label
};

