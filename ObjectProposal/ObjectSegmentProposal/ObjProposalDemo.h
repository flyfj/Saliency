

#pragma once

#include "ObjectRanker.h"
#include "Common/common_libs.h"
#include "IO/Kinect/KinectDataMan.h"
#include "Processors/Saliency/BingObjectness.h"

//////////////////////////////////////////////////////////////////////////
// general video demo components
// 1. raw data input
// 2. processing
// 3. visualization
//////////////////////////////////////////////////////////////////////////
class ObjProposalDemo
{
private:


public:
	ObjProposalDemo(void);

	bool RunObjWinProposal();

	bool RunSaliency(visualsearch::processors::attention::SaliencyType saltype);

};

