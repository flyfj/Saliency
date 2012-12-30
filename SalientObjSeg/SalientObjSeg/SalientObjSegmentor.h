//////////////////////////////////////////////////////////////////////////
//	salient object segmentor
//	fengjie@cis.pku
//	2011-9-1
//////////////////////////////////////////////////////////////////////////

#pragma once

#include <opencv2\opencv.hpp>
#include <vector>
#include <algorithm>
#include <cmath>
#include <io.h>
#include <direct.h>
#include <boost/property_map/property_map.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_matrix.hpp>
#include <boost/graph/floyd_warshall_shortest.hpp>
#include <boost/graph/johnson_all_pairs_shortest.hpp>

// mrf files
#include "mrf/mrf.h"
#include "mrf/GCoptimization.h"
// composition cost
#include "CompositionCost.h"

using namespace std;
using namespace cv;
using namespace boost;


//////////////////////////////////////////////////////////////////////////
// patch structure
struct Patch
{
	int id;	// start from 0
	Rect box;
	float bgScore;	//background score
	Point loc;	// patch coordinates (x,y)
};

struct SegmentResult 
{
	// data
	Mat mask;
	int area;
	Mat obj_img;
	// mrf params
	float bias;
	float tradeoff;
	// mrf energy
	float data_energy;
	float smooth_energy;
	float total_energy;
	// rank
	float CC_score;
	bool m_bValid;

	SegmentResult(): m_bValid(true) {}
};

//////////////////////////////////////////////////////////////////////////
class SalientObjSegmentor
{
public:
	// MRF params
	int numberOfLabels;
	vector<float> tradeoffs;
	vector<float> biases;
	float fgSeedTh;	// 0~1
	float bgSeedTh;	// 0~1
	// process params
	float m_fSegMinAreaRatio, m_fSegMaxAreaRatio;	// minimum/maximum valid segment area
	float m_fNmsTh;			// threshold to filter (intersection/union)
	// foreground weight map
	vector<vector<float>> patch2boundary;
	vector<Mat> fgWeightmaps;	//	for pixel
	vector<Mat> spFGWeightmaps;	//	for superpixel
	vector<Mat> bgMasks;		//	for different background settings
	float m_fBGTh;				//	if < th, then bg
	Point m_patchSize;	//	smaller, more accurate
	Point patchDim;
	vector<vector<Patch>> m_patches;
	vector<vector<float>> patchDistanceMap;
	// composition cost
	CompositionCost m_CC;
	// result data
	vector<SegmentResult> segmentRes;
	string m_savePath;
	string m_imgname;
	int m_dPropNum;		// proposal number: top n
	// static shared data
	static int width;	// image width
	static int height;	// image height
	static Mat cur_spFGWeightmap;	// 4 superpixel
	static Mat seedMask;	// mask for fg seed
	static Mat bgMask;		// mask for bg seed
	static Mat norm_img;	// color image
	static float tradeoff;	// lamda for smooth term
	static float bias;	// for data term

public:
	// common
	SalientObjSegmentor(void);
	~SalientObjSegmentor(void);

	// init: called every time to process a new image;
	// if fail to create save dir, return false and should
	// not process current image
	// save_path should end with "\\"
	bool Init(const Mat& img, string save_path, string imgname);

	void ClearAll();

	// create patches and compute pairwise distance map
	void ComputePairwiseShortestPathCost(const Mat& img);
	// compute shortest-path-map as foreground weighting map
	void ComputeFGWeightMap(const Mat& img);

	// segmentation
	void RunSegmentation(const Mat& img);

	// rank function
	static bool rank_by_cc(const SegmentResult& a, const SegmentResult& b) { return a.CC_score>b.CC_score; }
	static bool rank_by_area(const SegmentResult& a, const SegmentResult& b) { return a.area>b.area; }
};

//////////////////////////////////////////////////////////////////////////
// mrf term function
MRF::CostVal dCost(int pix, int i);

MRF::CostVal fnCost(int pix1, int pix2, int i, int j);