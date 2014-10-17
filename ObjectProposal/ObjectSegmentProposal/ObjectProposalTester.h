//////////////////////////////////////////////////////////////////////////
// various testing function: higher level task
// jiefeng@2014-10-04
//////////////////////////////////////////////////////////////////////////


#pragma once

#include "ObjectRanker.h"
#include "Segmentor3D.h"
#include "ObjProposalDemo.h"
#include "Common/common_libs.h"
#include "IO/Dataset/RGBDECCV14.h"
#include "IO/Dataset/Berkeley3DDataManager.h"

using namespace visualsearch;
using namespace visualsearch::io;
using namespace visualsearch::processors;

class ObjectProposalTester {

public:
	ObjectProposalTester() {
		nyu_cfn = "E:\\Datasets\\RGBD_Dataset\\NYU\\Depth2\\211.jpg";
		nyu_dfn = "E:\\Datasets\\RGBD_Dataset\\NYU\\Depth2\\211_d.png";
		uw_cfn = "E:\\Datasets\\RGBD_Dataset\\UW\\rgbd-scenes-v2_imgs\\imgs\\scene_02\\00001-color.png";
		uw_dfn = "E:\\Datasets\\RGBD_Dataset\\UW\\rgbd-scenes-v2_imgs\\imgs\\scene_02\\00001-depth.png";
		eccv_cfn = "E:\\Datasets\\RGBD_Dataset\\Saliency\\RGB\\8_08-34-01.jpg";
		eccv_dfn = "E:\\Datasets\\RGBD_Dataset\\Saliency\\Depth\\smoothedDepth\\8_08-34-01_Depth.png";
	}

	void TestRankerLearner();

	void BatchProposal();

	void TestSegmentor3D();

	void TestBoundaryClf(bool ifTrain);

	void Random();

private:

	string nyu_cfn;
	string nyu_dfn;
	string uw_cfn;
	string uw_dfn;
	string eccv_cfn;
	string eccv_dfn;

};

//////////////////////////////////////////////////////////////////////////

