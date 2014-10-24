

#pragma once

#include "Common/common_libs.h"
#include "Common/Tools/Tools.h"
#include "Common/Tools/ImgVisualizer.h"
#include "search/hashing/basehasher.h"
#include "search/hashing/lshcoder.h"
using namespace visualsearch::common;
using namespace visualsearch::common::tools;
using namespace visualsearch::search::binarycodes;


class ObjViewMatcher
{
public:
	ObjViewMatcher(void);

	bool PrepareDatabase();

	bool MatchView(const Mat& color_view);

private:
	bool ExtractViewFeat(const Mat& color_view, Mat& view_feat);
	
	ObjectCategory obj_db;
	string root_dir;

	LSHCoder lsh_coder;

};

