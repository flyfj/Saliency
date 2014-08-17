//////////////////////////////////////////////////////////////////////////
// an aggregator for various saliency detector
// jiefeng@2014-08-16
//////////////////////////////////////////////////////////////////////////


#pragma once

#include "../../Common/common_libs.h"
#include "GlobalContrastSaliency.h"

namespace visualsearch
{
	namespace processors
	{
		namespace saliency
		{

			class SaliencyComputer
			{
			private:

				GlobalContrastSaliency gcsaliency;

				//////////////////////////////////////////////////////////////////////////
				// some simple saliency methods
				bool ComputeFT(const Mat& cimg, Mat& salmap);
				bool ComputeSR(const Mat& cimg, Mat& salmap);
				bool ComputeLC(const Mat& cimg, Mat& salmap);
				bool ComputeHC(const Mat& cimg, Mat& salmap);
				bool ComputeRC(const Mat& cimg, Mat& salmap);

			public:
				SaliencyComputer(void);

				bool ComputeSaliencyMap(const Mat& cimg, SaliencyType saltype, Mat& salmap);
			};
		}
	}
}



