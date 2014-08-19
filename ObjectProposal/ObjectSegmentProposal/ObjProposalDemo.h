

#pragma once

#include "ObjectRanker.h"
#include "Common/common_libs.h"
//#include "IO/Kinect/KinectDataMan.h"
#include "IO/OpenCVCameraIO.h"
#include "Processors/Saliency/BingObjectness.h"
#include "Common/ImgVisualizer.h"
#include "Processors/nms.h"
#include "IterativeSegmentor.h"

//////////////////////////////////////////////////////////////////////////
// general video demo components
// 1. raw data input
// 2. processing
// 3. visualization
//////////////////////////////////////////////////////////////////////////

enum DemoType
{
	DEMO_SAL,
	DEMO_OBJECT_WIN,
	DEMO_OBJECT_SEG
};

using namespace visualsearch::processors::attention;

class ObjProposalDemo
{
private:
	visualsearch::processors::attention::BingObjectness bing;
	visualsearch::processors::attention::ObjectRanker ranker;
	visualsearch::visualization::ImgVisualizer imgvis;
	visualsearch::processors::attention::SaliencyComputer salcomputer;
	objectproposal::IterativeSegmentor iterSegmentor;

	int frameid;

public:
	ObjProposalDemo(void);

	bool RunObjSegProposal(Mat& cimg, Mat& dmap);

	bool RunObjWinProposal(Mat& cimg, Mat& dmap);

	bool RunSaliency(Mat& cimg, Mat& dmap, visualsearch::processors::attention::SaliencyType saltype);

	bool RunVideoDemo(DemoType dtype);

};

