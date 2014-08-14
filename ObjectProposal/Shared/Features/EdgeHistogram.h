//////////////////////////////////////////////////////////////////////////
// edge histogram
// jiefeng@13-12-08
//////////////////////////////////////////////////////////////////////////


#pragma once

#include "../Common/common_libs.h"
#include "../Common/Tools.h"

namespace visualsearch
{
	namespace features
	{
		struct EdgeFeatParams 
		{
			float MAG_TH;	// minimum magnitude of gradient to consider
			vector<int> multi_bin_num;	// for multi-scale histogram / single scale

			EdgeFeatParams()
			{
				MAG_TH = 0.05f;
				int binnums[] = {10, 15, 20};
				multi_bin_num = vector<int>(binnums, binnums+3);
			}
		};

		// a descriptor similar with hog but in global sense
		class EdgeHistogram
		{
		private:

			EdgeFeatParams eparams;

		public:
			EdgeHistogram(void);

			void Init(const EdgeFeatParams& params);

			bool Compute(const cv::Mat& gray_img, cv::Mat& edgehist, const cv::Mat& mask = cv::Mat());
		};
	}
}



