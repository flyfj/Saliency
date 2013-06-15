#pragma once

#include "GraphBasedSegment/wrapper.h"
#include "DecisionTree.h"
#include <opencv2/opencv.hpp>


class SaliencyLearner
{
private:
	DecisionTree dtree;


public:
	SaliencyLearner(void);
	~SaliencyLearner(void);
};

