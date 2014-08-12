//////////////////////////////////////////////////////////////////////////
// perform iterative saliency segmentation
// jiefeng @ 2012.12.11
//////////////////////////////////////////////////////////////////////////


#pragma once


#include "common_libs.h"
#include "ImageSegmentor.h"

namespace objectproposal
{
	using namespace visualsearch;

	// object segment proposal by iteratively merging superpixels
	// in every iteration, merge most similar connected sps and add a new sp,
	// update neighborhood relations
	class IterativeSegmentor
	{

	private:

		typedef map<float, Point> SimPair;	// small id first
		SimPair sp_pairs;	// generated candidate merge pairs
		map<int, SuperPixel> sps;	// generated sps
		Mat lab_img;
		int prim_seg_num;	// primitive superpixel number
		int cur_seg_id;
		vector<bool> bg_sign;

	public:

		// processor
		ImageSegmentor img_segmentor;

		IterativeSegmentor(void);

		int GetSegIdByLocation(Point pt) { return img_segmentor.m_idxImg.at<int>(pt.y, pt.x); }

		// init data and processors
		void Init();

		// main process
		bool Run(const Mat& cimg);

		// compute features for superpixels
		bool ComputeSPFeatures(const Mat& cimg, SuperPixel& sp);

		float ComputeSPDist(const SuperPixel& sp1, const SuperPixel& sp2);

		bool DoMergeIteration(const Mat& cimg, bool verbose = false);


	};
}


