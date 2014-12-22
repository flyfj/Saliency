//////////////////////////////////////////////////////////////////////////
// main proposal class
// jiefeng@2014-08-15
//////////////////////////////////////////////////////////////////////////


#pragma once

#include "stdafx.h"
#include "ObjectRanker.h"

namespace objectproposal
{
	using namespace visualsearch::common;
	using namespace visualsearch::processors;
	using namespace visualsearch::processors::segmentation;

	class ObjSegmentProposal
	{
	public:
		ObjSegmentProposal(void);

		bool Run(const Mat& cimg, const Mat& dmap, int topK, vector<VisualObject>& res);

		bool GetCandidatesFromIterativeSeg(const Mat& cimg, const Mat& dmap, vector<VisualObject>& sps);
		//bool GetCandidatesFromSegment3D(const Mat& cimg, const Mat& dmap, vector<VisualObject>& sps);

		bool GetCandidatesFromSaliency(const Mat& cimg, const Mat& dmap, vector<VisualObject>& sps);
		//////////////////////////////////////////////////////////////////////////

		bool VisProposals(const Mat& cimg, const vector<VisualObject>& res);

		//////////////////////////////////////////////////////////////////////////
		// evaluation
		void ComputePRCurves(const vector<VisualObject>& ranked_objs, const vector<Mat>& gt_masks, float cover_th, 
			vector<Point2f>& pr_vals, vector<Point3f>& best_overlap, bool seg_or_win = true);

		bool verbose;

	private:
		visualsearch::processors::segmentation::IterativeSegmentor iter_segmentor;
		visualsearch::processors::attention::ObjectRanker seg_ranker;
		visualsearch::processors::segmentation::SegmentProcessor seg_proc;
		//Segmentor3D seg3d;
	};
}



