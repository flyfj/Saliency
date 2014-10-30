

#pragma once

#include "Common/common_libs.h"
#include "Common/Tools/Tools.h"
#include "Common/Tools/ImgVisualizer.h"
#include "search/hashing/basehasher.h"
#include "search/hashing/lshcoder.h"
using namespace visualsearch::common;
using namespace visualsearch::common::tools;
using namespace visualsearch::features;
using namespace visualsearch::search::binarycodes;


class ObjViewMatcher
{
public:
	ObjViewMatcher(void);

	// get object metadata: filepath
	bool PrepareDatabase();

	bool MatchView(const Mat& color_view, const Mat& depth_view);

private:
	bool ExtractViewFeat(const Mat& color_view, const Mat& dmap_view, Mat& view_feat);

	bool LearnOptimalBinaryCodes(int code_len);

	// cost function
	float EvaluateCodeQuality();
	
	ObjectCategory obj_db;
	string root_dir;

	LSHCoder lsh_coder;
	Feature3D feat3d;

};

