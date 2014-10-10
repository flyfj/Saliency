//////////////////////////////////////////////////////////////////////////
// handles 3d features
// jiefeng@2014-10-10
//////////////////////////////////////////////////////////////////////////


#pragma once

#include "common3d.h"
#include "PCLManager.h"
#include "Common/common_libs.h"
#include "Common/Tools/RGBDTools.h"
using namespace visualsearch::common;
using namespace visualsearch::common::tools;


class Feature3D
{
public:
	Feature3D(void);

	bool ComputeNormals(const Mat& pts3d);

	void ComputeNormals(SuperPixel& sp);

	void ComputeBoundaryMaps(const Mat& cimg, const Mat& dmap);

private:
	
};

