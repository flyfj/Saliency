

#pragma once

#include "Common/common_libs.h"
#include "Common/Tools/Tools.h"
#include "Common/Tools/ImgVisualizer.h"
#include "Features/Feature3D.h"
#include "Features/Texture/EdgeHistogram.h"
#include "search/hashing/basehasher.h"
#include "search/hashing/lshcoder.h"
#include "Processors/SlidingWinDetector.hpp"
using namespace visualsearch::common;
using namespace visualsearch::common::tools;
using namespace visualsearch::features;
using namespace visualsearch::features::texture;
using namespace visualsearch::search::binarycodes;
using namespace visualsearch::processors;


class ObjViewMatcher
{
public:
	ObjViewMatcher(void);

	// get object metadata: filepath
	bool PrepareDatabase();

	bool MatchView(const Mat& view_feat, vector<DMatch>& matches);

	bool SearchImage(const Mat& cimg, const Mat& dmap_raw);

private:
	bool ExtractViewFeat(const Mat& color_view, const Mat& dmap_view, Mat& view_feat);

	bool LearnOptimalBinaryCodes(int code_len);

	bool ComputeGradient(const Mat& color_view, Mat& grad_view);

	// cost function
	float EvaluateCodeQuality();
	
	ObjectCategory obj_db;
	string root_dir;

	EdgeHistogram edge_proc;
	LSHCoder lsh_coder;
	Feature3D feat3d;
	Size view_sz;

};

