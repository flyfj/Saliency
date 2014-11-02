//////////////////////////////////////////////////////////////////////////

#pragma once

#include "Common/common_libs.h"
#include "Features/Feature3D.h"
using namespace visualsearch;

class ObjectSearcher
{
public:
	ObjectSearcher(void);

	bool SplitSearch(const Mat& cimg, const Mat& dmap);

	bool SplitSearchIteration(int sp_id, const Mat& cimg, const Mat& pts3d_map, const Mat& normal_map);

	vector<SuperPixel> sps;

private:

};

