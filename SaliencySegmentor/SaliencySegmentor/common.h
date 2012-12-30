//////////////////////////////////////////////////////////////////////////
//	common structures
//////////////////////////////////////////////////////////////////////////


#pragma once


#include <opencv2/opencv.hpp>
#include <set>
using namespace std;
using namespace cv;


namespace Saliency
{

	/************************************************************************/
	/* Structures                                                           */
	/************************************************************************/

	// superpixel feature
	struct SegSuperPixelFeature
	{
		SegSuperPixelFeature() : bnd_pixels(0), perimeter(0), 
			area(0), id(-1)
		{ box_pos[0].x = 100000; box_pos[0].y = 100000; box_pos[1].x = 0; box_pos[1].y = 0; }

		vector<float> feat;	// feature vector

		Point box_pos[2];	// bounding box: top-left, bottom-right
		Rect box;	// bounding box
		float bnd_pixels;	// boundary pixel numbers
		float perimeter;
		float area;
		Point2f centroid;
		set<int> neighbor_ids;
		unsigned int id;
		Mat mask;	// currently, same size as image for each superpixel

		static bool InsideSegment(const Point& pt, const SegSuperPixelFeature& sp_feat)
		{
			if(sp_feat.mask.empty() || !sp_feat.box.contains(pt))
				return false;
			
			if( sp_feat.mask.at<uchar>(pt) > 0 )
				return true;
			else
				return false;
		}

	};

	//used in paper: L_A_B
	const int quantBins[3] = {4, 8, 8}; 

	const bool use4Neighbor = true;

}
