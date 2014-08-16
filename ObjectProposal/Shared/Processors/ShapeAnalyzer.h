//////////////////////////////////////////////////////////////////////////
// shape processor
// jiefeng@2014-08-15
//////////////////////////////////////////////////////////////////////////


#pragma once

#include "Common/common_libs.h"

namespace visualsearch
{
	namespace processors
	{
		class ShapeAnalyzer
		{
		private:


		public:
			ShapeAnalyzer(void);

			bool FloodFillMask(const cv::Mat& grayimg, cv::Point& seed, float loDiff, float upDiff, cv::Mat& mask);

			bool ExtractConnectedComponents(const Mat& grayimg, vector<BasicShape>& shapes);
		};
	}
}



