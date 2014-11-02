#include "ObjectSearcher.h"


ObjectSearcher::ObjectSearcher(void)
{
}


bool ObjectSearcher::SplitSearchIteration(int sp_id, const Mat& cimg, const Mat& dmap, const Mat& normal_map) {


	return true;
}

bool ObjectSearcher::SplitSearch(const Mat& cimg, const Mat& dmap) {
	Mat lab_img;
	cvtColor(cimg, lab_img, CV_BGR2Lab);
	Mat dmap_color;
	dmap.convertTo(dmap_color, CV_32F);

	features::Feature3D feat3d;
	Mat pts3d_map, normal_map;
	feat3d.ComputeKinect3DMap(dmap, pts3d_map, true);
	feat3d.ComputeNormalMap(pts3d_map, normal_map);

	// convert to color
	normalize(dmap, dmap_color, 1, 0, NORM_MINMAX, CV_32F);
	dmap_color.convertTo(dmap_color, CV_8U, 255);
	pts3d_map.convertTo(pts3d_map, CV_8U, 255);
	normal_map.convertTo(normal_map, CV_8U, 255);

	SuperPixel all_sp;
	all_sp.mask.create(cimg.rows, cimg.cols, CV_8U);
	all_sp.mask.setTo(1);
	sps.push_back(all_sp);

	SplitSearchIteration(0, lab_img, pts3d_map, normal_map);

	

	return true;
}