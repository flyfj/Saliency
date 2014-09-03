//////////////////////////////////////////////////////////////////////////
// process segment, extract features
// jiefeng@2014-8-13
//////////////////////////////////////////////////////////////////////////


#pragma once

#include "Common/common_libs.h"
#include "Common/tools/Tools.h"
#include "Processors/Segmentation/ImageSegmentor.h"
#include "Features/Color/ColorDescriptors.h"
#include "Features/Texture/EdgeHistogram.h"
#include "Features/DepthDescriptors.h"


namespace visualsearch
{
	using namespace common;

	namespace processors
	{
		namespace segmentation
		{
			enum SPFeat
			{
				SP_COLOR = 1,
				SP_TEXTURE = 2,
				SP_DEPTH = 4
			};

			class SegmentProcessor
			{
			private:

				features::color::ColorDescriptors colorDesc;
				features::texture::EdgeHistogram edgeDesc;
				features::DepthDescriptors depthDesc;

			public:

				SegmentProcessor(void);

				bool ExtractBasicSegmentFeatures(SuperPixel& sp, const Mat& cimg, const Mat& dmap);

				bool ExtractSegmentFeatures(SuperPixel& sp, const Mat& cimg, const Mat& dmap, int feattype);

				static float ComputeSegmentDist(const SuperPixel& sp1, const SuperPixel& sp2);
			};
		}
	}
	
}


