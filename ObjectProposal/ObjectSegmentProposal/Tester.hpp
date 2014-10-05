//////////////////////////////////////////////////////////////////////////
// various testing function: higher level task
// jiefeng@2014-10-04
//////////////////////////////////////////////////////////////////////////


#pragma once

#include "ObjectRanker.h"
using namespace visualsearch;
using namespace visualsearch::io;
using namespace visualsearch::processors;

class ObjectProposalTester {

public:
	void TestRankerLearner();

private:


};

//////////////////////////////////////////////////////////////////////////

void ObjectProposalTester::TestRankerLearner() {
	processors::attention::ObjectRanker ranker;
	ranker.PrepareRankTrainData(dataset::DB_SALIENCY_RGBD);
	ranker.LearnObjectPredictor();
}