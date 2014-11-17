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
#include "Features/Color/ColorDescriptors.h"
#include "IO/Dataset/Berkeley3DDataManager.h"
#include "Processors/Segmentation/ImageSegmentor.h"
#include "Processors/Segmentation/SegmentProcessor.h"
#include "Processors\Segmentation\Segmentor3D.h"
#include "ObjSegmentProposal.h"
#include "Learners/RandomTrees/DecisionTree.h"
#include <Eigen/Eigen>
#include "ObjViewMatcher.h"
#include "Processors/Attention/SaliencyComputer.h"
#include "Processors/Segmentation/PointSegmentator.h"
#include "ObjectSearcher.h"
#include "SuperpixelClf.h"
#include "ObjPatchMatcher.h"

using namespace visualsearch;
using namespace visualsearch::io;
using namespace visualsearch::processors;
using namespace visualsearch::processors::attention;
using namespace visualsearch::features;

class ObjectProposalTester {

public:
	ObjectProposalTester() {
		nyu_cfn = "E:\\Datasets\\RGBD_Dataset\\NYU\\Depth2\\57.jpg";
		nyu_dfn = "E:\\Datasets\\RGBD_Dataset\\NYU\\Depth2\\57_d.png";
		nyu_gtfn = "E:\\Datasets\\RGBD_Dataset\\NYU\\Depth2\\444_l.png";
		
		uw_cfn = "E:\\Datasets\\RGBD_Dataset\\UW\\rgbd-scene-dataset1\\table_small\\table_small_1\\";
		uw_dfn = "E:\\Datasets\\RGBD_Dataset\\UW\\rgbd-scene-dataset1\\table_small\\table_small_1\\";
		uw_gt_dir = "E:\\Datasets\\RGBD_Dataset\\UW\\rgbd-scene-dataset1\\table_small\\gt1\\";
		
		uw_obj_cfn = "E:\\Datasets\\RGBD_Dataset\\UW\\rgbd-obj-dataset\\rgbd-dataset\\stapler\\stapler_1\\stapler_1_1_1_crop.png";
		uw_obj_dfn = "E:\\Datasets\\RGBD_Dataset\\UW\\rgbd-obj-dataset\\rgbd-dataset\\stapler\\stapler_1\\stapler_1_1_1_depthcrop.png";
		
		eccv_cfn = "E:\\Datasets\\RGBD_Dataset\\Saliency\\RGB\\3_09-48-48.jpg";
		eccv_dfn = "E:\\Datasets\\RGBD_Dataset\\Saliency\\Depth\\smoothedDepth\\3_09-48-48_Depth.png";

		nyu20_cdir = "E:\\Datasets\\RGBD_Dataset\\nyu_20\\rgb\\";
		nyu20_ddir = "E:\\Datasets\\RGBD_Dataset\\nyu_20\\depth\\";
		nyu20_gtdir = "E:\\Datasets\\RGBD_Dataset\\nyu_20\\mask\\";

		save_dir = "E:\\res\\segments\\cvpr15\\";
	}

	void TestRankerLearner();

	void BatchProposal();

	void BoundaryPlayground();

	void TestSegmentor3D();

	void TestBoundaryClf(bool ifTrain);

	void EvaluateOnDataset(DatasetName db_name);

	void TestSegment();
	void Random();

	void TestObjSearch();

	void TestSuperpixelClf(bool ifTrain);

	void TestSaliency();

	void TestViewMatch();

	void TestGraphcut();

	void TestPatchMatcher();

	void RunWatershed(const Mat& cimg, Mat& markers, int region_num, vector<SuperPixel>& sps);

	void Build3DPCL(DatasetName db_name);

private:
	bool LoadNYU20Masks(FileInfo imgfn, vector<Mat>& gt_masks);

	float ComputeSaliencyMapEntropy(const Mat& sal_map);

	string nyu_cfn;
	string nyu_dfn;
	string nyu_gtfn;
	string nyu20_cdir;
	string nyu20_ddir;
	string nyu20_gtdir;
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

