

#pragma once

#include "Common/common_libs.h"
#include "Common/Tools/Tools.h"
#include "Common/Tools/ImgVisualizer.h"
#include "Features/Feature3D.h"
#include "Features/Keypoints/ImgKeypointDetector.h"
#include "Features/Texture/EdgeHistogram.h"
#include "Features/Texture/HoGComputer.h"
#include "search/hashing/basehasher.h"
#include "search/hashing/lshcoder.h"
#include "Processors/SlidingWinDetector.hpp"
using namespace visualsearch::common;
using namespace visualsearch::common::tools;
using namespace visualsearch::features;
using namespace visualsearch::features::texture;
using namespace visualsearch::features::keypoints;
using namespace visualsearch::search::binarycodes;
using namespace visualsearch::processors;


class ObjViewMatcher
{
public:
	ObjViewMatcher(void);

	// get object metadata: filepath
	bool PrepareDatabase();

	bool MatchView(const Mat& view_feat, vector<DMatch>& matches);

	bool MatchViewByPatches(const MatFeatureSet& view_maps, vector<DMatch>& matches);

	bool SearchImage(const Mat& cimg, const Mat& dmap_raw);

private:
	bool ExtractViewFeat(const Mat& color_view, const Mat& dmap_view, Mat& view_feat);

	bool ComputeGradient(const Mat& color_view, Mat& grad_view);

	/**
	 *  patch related methods
	 */
	bool SelectPatches(const MatFeatureSet& view_maps, vector<Point>& patch_locs);

	ObjectCategory obj_db;
	string root_dir;

	ImgKeypointDetector key_detector;
	EdgeHistogram edge_proc;
	HoGComputer hog;
	LSHCoder lsh_coder;
	Feature3D feat3d;
	Size view_sz;
	Size patch_sz;

};

