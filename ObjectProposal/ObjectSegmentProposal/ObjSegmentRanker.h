//////////////////////////////////////////////////////////////////////////
// ranker for candiate object segments
// jiefeng@2014-08-12
//////////////////////////////////////////////////////////////////////////


#pragma once

#include "Common/common_libs.h"
#include "Processors/ImageSegmentor.h"
#include "Common/Tools.h"
#include "Features/ColorDescriptors.h"
#include "Features/DepthDescriptors.h"


namespace objectproposal
{
	using namespace visualsearch;

	// use saliency or prior to rank object segment
	class ObjSegmentRanker
	{
	private:
		float ComputeCenterSurroundColorContrast(const Mat& cimg, const SuperPixel& sp);

		features::ColorDescriptors colordesc;

	public:
		ObjSegmentRanker(void);


		
	};
}


