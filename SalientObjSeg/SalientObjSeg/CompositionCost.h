//////////////////////////////////////////////////////////////////////////
//	class used to compute composition cost as saliency measure
//	for a given segment
//
//	fengjie@cis.pku
//	2011-9-6
//////////////////////////////////////////////////////////////////////////

#pragma once

#include "ImageSegmentor.h"
#include <opencv2\opencv.hpp>
#include <vector>
#include <set>
#include <iostream>
using namespace std;
using namespace cv;


struct SegSuperPixelFeature
{
	SegSuperPixelFeature() : composition_cost(0.f)
		,extendedArea(0), leftInnerArea(0), leftOuterArea(0)
		,dist_to_win(0)
	{}

	void Init(const Superpixel& sp, const Mat& lab_img);

	float extendedArea;

	vector<float> feat;
	vector<TPair> pairs;

	float leftInnerArea, leftOuterArea;	// inside and outside areas with respect to a mask
	float composition_cost;
	vector<float> fgWeights;
	float currentArea;	// area need to be filled currently
	int id;
	vector<TPair> composers;	// segments composing current sp

	// compose function
	inline void InitFillArea(int area_in_mask)
	{		
		assert(area_in_mask > 0);
		leftInnerArea = area_in_mask;
		leftOuterArea = extendedArea - leftInnerArea;

		if (leftInnerArea > leftOuterArea)
		{
			leftInnerArea -=  leftOuterArea;
			leftOuterArea = 0;
		}
		else
		{
			leftInnerArea = 0;
			leftOuterArea -= leftInnerArea;
		}

		currentArea = leftInnerArea;
	}

	inline void InitFillArea()
	{
		leftInnerArea = 0;
		leftOuterArea = extendedArea;

		currentArea = 0;
	}

	static float FeatureIntersectionDistance(const SegSuperPixelFeature& a, const SegSuperPixelFeature& b)
	{
		float dist = 0;
		for(size_t i = 0; i < a.feat.size(); i++)
			dist += ((a.feat[i] < b.feat[i]) ? a.feat[i] : b.feat[i]);
		return 1 - dist;
	}

	float dist_to_win;
	static bool comp_by_dist(const SegSuperPixelFeature* a, const SegSuperPixelFeature* b)	{	return a->dist_to_win > b->dist_to_win;	}
	// x: id, y: area
	static bool comp_by_area(const Point a, const Point b) { return a.y > b.y; }
};


class CompositionCost
{
public:
	// segmentor
	ImageSegmentor m_segmentor;

	// data
	Mat m_img;
	float m_fMaxCost;
	vector<SegSuperPixelFeature> sp_features;	// id consistent with segments
	

public:
	CompositionCost(void) {}
	~CompositionCost(void) {}

	// every time change the image, need to re-call init
	void Init(const Mat& img);

	void ClearAll();

	// compute composition cost for a segment within the context of a image
	float ComputeCompositionCost(const Mat& mask);
};
