

#pragma once

#include "ObjSegmentProposal.h"
//#include "Processors/Attention/BingObjectness.h"
#include "ObjectRanker.h"
//#include "KinectDataMan.h"

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
	DEMO_OBJECT_SEG,
	DEMO_VIEW_ONLY
};

using namespace visualsearch::common;
using namespace visualsearch::io::camera;


class ObjProposalDemo
{
private:
	//visualsearch::processors::attention::BingObjectness bing;
	visualsearch::processors::attention::ObjectRanker ranker;
	visualsearch::tools::ImgVisualizer imgvis;
	visualsearch::processors::attention::SaliencyComputer salcomputer;
	visualsearch::processors::segmentation::IterativeSegmentor iterSegmentor;
	objectproposal::ObjSegmentProposal seg_proposal;
	static void mouse_callback(int event, int x, int y, int flags, void*);
	static Point mouse_point;

	string DATADIR;

	int frameid = 0;

public:

	ObjProposalDemo(void);

	bool RunObjSegProposal(string fn, Mat& cimg, Mat& dmap, Mat& oimg);

	//bool RunObjWinProposal(Mat& cimg, Mat& dmap);

	bool RunSaliency(Mat& cimg, Mat& dmap, visualsearch::processors::attention::SaliencyType saltype);

	bool RunVideoDemo(SensorType stype, DemoType dtype);

};

