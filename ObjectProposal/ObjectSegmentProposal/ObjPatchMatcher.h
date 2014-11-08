//////////////////////////////////////////////////////////////////////////

#pragma once

#include "Common/common_libs.h"
#include "Common/Tools/Tools.h"
#include "Search/Searcher.h"
#include "Features/Feature3D.h"
#include "IO/Dataset/NYUDepth2DataMan.h"
#include "Common/Tools/ImgVisualizer.h"
using namespace visualsearch;
using namespace visualsearch::features;
using namespace visualsearch::common;
using namespace visualsearch::search;
using namespace visualsearch::io::dataset;


#define VERBOSE

class ObjPatchMatcher
{
public:
	ObjPatchMatcher(void);

	bool PreparePatchDB(DatasetName db_name);

	bool Match(const Mat& cimg, const Mat& dmap_raw);

private:
	bool ComputePatchFeat(const Mat& patch, Mat& feat);

	bool MatchPatch(const Mat& feat, int k, vector<DMatch>& res);

	Searcher searcher;
	Size patch_size;
	Mat patch_data;
	ObjectCategory patch_meta;
	vector<bool> valid_cls;
};

