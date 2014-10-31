//////////////////////////////////////////////////////////////////////////
// main proposal class
// jiefeng@2014-08-15
//////////////////////////////////////////////////////////////////////////


#pragma once

#include "Processors\Segmentation\IterativeSegmentor.h"
#include "Processors\Segmentation\SegmentProcessor.h"
#include "Processors\Segmentation\Segmentor3D.h"
#include "Common\common_libs.h"
#include "ObjectRanker.h"
#include "Common/Tools/RGBDTools.h"

namespace objectproposal
{
	using namespace visualsearch::common;
	using namespace visualsearch::processors::segmentation;

	class ObjSegmentProposal
	{
	public:
		ObjSegmentProposal(void);

		bool Run(const Mat& cimg, const Mat& dmap, int topK, vector<SuperPixel>& res);

		bool GetCandidatesFromIterativeSeg(const Mat& cimg, const Mat& dmap, vector<SuperPixel>& sps);
		bool GetCandidatesFromSegment3D(const Mat& cimg, const Mat& dmap, vector<SuperPixel>& sps);

		//////////////////////////////////////////////////////////////////////////

		bool VisProposals(const Mat& cimg, const vector<SuperPixel>& res);

		//////////////////////////////////////////////////////////////////////////
		// evaluation
		void ComputePRCurves(const vector<SuperPixel>& ranked_objs, const vector<Mat>& gt_masks, float cover_th, 
			vector<Point2f>& pr_vals, vector<Point3f>& best_overlap, bool seg_or_win = true);

	private:
		visualsearch::processors::segmentation::IterativeSegmentor iter_segmentor;
		visualsearch::processors::attention::ObjectRanker seg_ranker;
		Segmentor3D seg3d;
	};
}



