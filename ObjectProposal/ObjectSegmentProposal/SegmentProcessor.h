//////////////////////////////////////////////////////////////////////////
// process segment, extract features
// jiefeng@2014-8-13
//////////////////////////////////////////////////////////////////////////


#pragma once

#include "Common/common_libs.h"
#include "Common/Tools.h"
#include "Processors/ImageSegmentor.h"
#include "Features/ColorDescriptors.h"
#include "Features/EdgeHistogram.h"
#include "Features/DepthDescriptors.h"


namespace objectproposal
{
	using namespace visualsearch;

	enum SPFeat
	{
		SP_COLOR = 1,
		SP_TEXTURE = 2,
		SP_DEPTH = 4
	};

	class SegmentProcessor
	{
	private:

		features::ColorDescriptors colorDesc;
		features::EdgeHistogram edgeDesc;
		features::DepthDescriptors depthDesc;

	public:

		SegmentProcessor(void);

		bool ExtractBasicSegmentFeatures(SuperPixel& sp, const Mat& cimg, const Mat& dmap);

		bool ExtractSegmentFeatures(SuperPixel& sp, const Mat& cimg, const Mat& dmap, int feattype);

		static float ComputeSegmentDist(const SuperPixel& sp1, const SuperPixel& sp2);
	};
}


