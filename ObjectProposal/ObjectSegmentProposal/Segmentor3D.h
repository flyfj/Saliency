//////////////////////////////////////////////////////////////////////////
// jiefeng©2014-10-13
//////////////////////////////////////////////////////////////////////////

#pragma once

#include "Common/common_libs.h"
#include "Common/Tools/ImgVisualizer.h"
#include "IO/Dataset/RGBDECCV14.h"
#include "IO/Dataset/NYUDepth2DataMan.h"
#include "IO/Dataset/DataManagerInterface.h"
#include "Features/Feature3D.h"
#include "Processors/Segmentation/SegmentProcessor.h"
#include "Learners/RandomTrees/DecisionTree.h"
using namespace visualsearch;
using namespace visualsearch::common;
using namespace visualsearch::io::dataset;

class FeatPoint {
public:
	Vec3f pts_vec;
	Vec3f normal_vec;
	Vec3f lab_vec;

	static float ComputeDist(const FeatPoint& a, const FeatPoint& b) {
		float pts_dist = norm(a.pts_vec, b.pts_vec, NORM_L2);
		return pts_dist;
		float normal_dist = MAX(1 - a.normal_vec.dot(b.normal_vec), 1);
		return sqrt( pts_dist*pts_dist + normal_dist*normal_dist );
	}

};

class Segmentor3D
{
public:
	Segmentor3D(void);

	bool RunRegionGrowing(const Mat& pts3d_bmap);
	bool Run(const vector<vector<FeatPoint>>& super_img);

	bool RunBFS(const vector<vector<FeatPoint>>& super_img);

	bool TrainBoundaryDetector(DatasetName db_name);
	// raw data or bmap?
	bool RunBoundaryDetection(const Mat& cimg, const Mat& dmap, Mat& bmap);

	Mat labels;
	float DIST_TH;

private:
	
	void BFS(int x, int y, int label, const vector<vector<FeatPoint>>& super_img);

};

