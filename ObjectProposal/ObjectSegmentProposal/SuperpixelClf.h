


#pragma once

#include "Common/common_libs.h"
#include "IO/Dataset/NYUDepth2DataMan.h"
#include "Learners/RandomTrees/RandomForest.hpp"
#include "Processors/Segmentation/SegmentProcessor.h"
#include "Processors/DescriptorDictGenerator.h"
using namespace visualsearch::common;
using namespace visualsearch::features;
using namespace visualsearch::io::dataset;
using namespace visualsearch::processors;
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

	bool Predict(const Mat& samp, vector<double>& scores);

	static float LabelDistributionDist(const vector<double>& label1, const vector<double>& label2);

private:
	RForestTrainingParams rfparams;
	RandomForest<LinearFeature> rf;
	SegmentProcessor seg_processor;
	DescriptorDictionaryGenerator dict_gen;
	string rf_model_file;
	string db_info_file;
	string sp_dict_file;
	int sp_feats_;
	map<int, int> label_map;	// label remapping, raw label -> new label
};

