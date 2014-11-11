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

	// extract small patch around object boundary
	bool PreparePatchDB(DatasetName db_name);

	// load view patches from uw
	bool PrepareViewPatchDB();

	// match boundary patch
	bool Match(const Mat& cimg, const Mat& dmap_raw);

	bool MatchViewPatch(const Mat& cimg, const Mat& dmap_raw);

	Size patch_size;
	bool use_depth;

private:
	bool ComputePatchFeat(MatFeatureSet& patches, Mat& feat);

	bool MatchPatch(const Mat& feat, int k, vector<DMatch>& res);

	Searcher searcher;

	Mat patch_data;
	ObjectCategory patch_meta;
	vector<bool> valid_cls;
	string uw_view_root;
};

