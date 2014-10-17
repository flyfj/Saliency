//////////////////////////////////////////////////////////////////////////
// main proposal class
// jiefeng@2014-08-15
//////////////////////////////////////////////////////////////////////////


#pragma once

#include "Processors\Segmentation\IterativeSegmentor.h"
#include "Processors\Segmentation\SegmentProcessor.h"
#include "Common\common_libs.h"
#include "ObjectRanker.h"
#include "Segmentor3D.h"
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

		//////////////////////////////////////////////////////////////////////////

		bool VisProposals(const Mat& cimg, const vector<SuperPixel>& res);

	private:
		bool GetCandidatesFromIterativeSeg(const Mat& cimg, const Mat& dmap, vector<SuperPixel>& sps);
		bool GetCandidatesFromSegment3D(const Mat& cimg, const Mat& dmap, vector<SuperPixel>& sps);

		visualsearch::processors::segmentation::IterativeSegmentor iter_segmentor;
		visualsearch::processors::attention::ObjectRanker seg_ranker;
		Segmentor3D seg3d;
	};
}



