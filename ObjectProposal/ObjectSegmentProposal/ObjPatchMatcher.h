//////////////////////////////////////////////////////////////////////////

#pragma once

#include "Common/common_libs.h"
#include "Common/Tools/Tools.h"
#include "Search/Searcher.h"
#include "Features/Feature3D.h"
#include "IO/Dataset/NYUDepth2DataMan.h"
#include "Common/Tools/ImgVisualizer.h"
#include "Search/Hashing/LSHCoder.h"
//#include "Processors/Segmentation/ObjectSegmentor.h"
using namespace visualsearch;
using namespace visualsearch::features;
using namespace visualsearch::common;
using namespace visualsearch::search;
using namespace visualsearch::search::binarycodes;
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
	bool use_code;

private:
	bool ComputePatchFeat(MatFeatureSet& patches, Mat& feat);

	bool MatchPatch(const Mat& feat, int k, vector<DMatch>& res);

	bool MatchCode(const HashKey& query_key, int k, vector<DMatch>& res);

	// if obj_pt_sign is 1, then points with positive in the line is object
	bool ComputeDominantLine(const Mat& mask_patch, Point tl_pt, Point3f& line_coeff, int& obj_pt_sign);

	// save or load patch data from file to avoid re-extraction
	bool PatchDataIO(bool toSave);

	Searcher searcher;

	Mat patch_data;
	ObjectCategory patch_meta;
	vector<HashKey> patch_keys;
	vector<bool> valid_cls;
	string uw_view_root;
};

