//////////////////////////////////////////////////////////////////////////
// jiefeng©2014-10-13
//////////////////////////////////////////////////////////////////////////

#pragma once

#include "Common/common_libs.h"

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

	bool Run(const vector<vector<FeatPoint>>& super_img);

	Mat labels;
	float DIST_TH;

private:
	

};

