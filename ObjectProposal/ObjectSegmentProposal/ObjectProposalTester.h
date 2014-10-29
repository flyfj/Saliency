//////////////////////////////////////////////////////////////////////////
// various testing function: higher level task
// jiefeng@2014-10-04
//////////////////////////////////////////////////////////////////////////


#pragma once

#include "ObjectRanker.h"
#include "ObjProposalDemo.h"
#include "Common/common_libs.h"
#include "IO/Dataset/RGBDECCV14.h"
#include "Common/Tools/RGBDTools.h"
#include "IO/Dataset/Berkeley3DDataManager.h"
#include "Processors/Segmentation/ImageSegmentor.h"
#include "Processors/Segmentation/SegmentProcessor.h"
#include "Processors\Segmentation\Segmentor3D.h"
#include "ObjSegmentProposal.h"
#include <Eigen/Eigen>
#include "ObjViewMatcher.h"

using namespace visualsearch;
using namespace visualsearch::io;
using namespace visualsearch::processors;
using namespace visualsearch::features;

class ObjectProposalTester {

public:
	ObjectProposalTester() {
		nyu_cfn = "E:\\Datasets\\RGBD_Dataset\\NYU\\Depth2\\2.jpg";
		nyu_dfn = "E:\\Datasets\\RGBD_Dataset\\NYU\\Depth2\\2_d.png";
		
		uw_cfn = "E:\\Datasets\\RGBD_Dataset\\UW\\rgbd-scene-dataset1\\table_small\\table_small_1\\";
		uw_dfn = "E:\\Datasets\\RGBD_Dataset\\UW\\rgbd-scene-dataset1\\table_small\\table_small_1\\";
		uw_gt_dir = "E:\\Datasets\\RGBD_Dataset\\UW\\rgbd-scene-dataset1\\table_small\\gt1\\";
		
		uw_obj_cfn = "E:\\Datasets\\RGBD_Dataset\\UW\\rgbd-obj-dataset\\rgbd-dataset\\stapler\\stapler_1\\stapler_1_1_1_crop.png";
		uw_obj_dfn = "E:\\Datasets\\RGBD_Dataset\\UW\\rgbd-obj-dataset\\rgbd-dataset\\stapler\\stapler_1\\stapler_1_1_1_depthcrop.png";
		
		eccv_cfn = "E:\\Datasets\\RGBD_Dataset\\Saliency\\RGB\\9_12-50-08.jpg";
		eccv_dfn = "E:\\Datasets\\RGBD_Dataset\\Saliency\\Depth\\smoothedDepth\\9_12-50-08_Depth.png";

		save_dir = "E:\\res\\segments\\meeting\\";
	}

	void TestRankerLearner();

	void BatchProposal();

	void BoundaryPlayground();

	void TestSegmentor3D();

	void TestBoundaryClf(bool ifTrain);

	void EvaluateOnDataset(DatasetName db_name);

	void TestSegment();
	void Random();

	void TestViewMatch();

private:
	string nyu_cfn;
	string nyu_dfn;
	string uw_cfn;
	string uw_dfn;
	string uw_obj_cfn;
	string uw_obj_dfn;
	string uw_gt_dir;
	string eccv_cfn;
	string eccv_dfn;
	string save_dir;

};

//////////////////////////////////////////////////////////////////////////

