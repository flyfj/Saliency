


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

	bool Train(DatasetName db_name);

	bool Predict(const Mat& cimg, const Mat& dmap);

	bool Predict(const SuperPixel& sp);

private:
	RForestTrainingParams rfparams;
	RandomForest<LinearFeature> rf;
	string rf_model_file;
};

