//////////////////////////////////////////////////////////////////////////
// depth features
// jiefeng@2014-8-13
//////////////////////////////////////////////////////////////////////////


#pragma once

#include "../Common/common_libs.h"

namespace visualsearch
{
	namespace features
	{
		enum DepthFeatType
		{
			DEPTH_FEAT_MEAN,
			DEPTH_FEAT_HIST
		};

		struct DepthFeatParams
		{
			// params
			DepthFeatType dtype;
		};

		// extract various depth related descriptors
		// assume a [0,1] normalized dmap
		class DepthDescriptors
		{
		private:
			
			DepthFeatParams params;

			//////////////////////////////////////////////////////////////////////////

			bool ComputeMeanDepth(const Mat& dmap, Mat& feat, const Mat& mask = Mat());

		public:
			DepthDescriptors(void);

			bool Init(const DepthFeatParams& dparams);

			bool Compute(const Mat& dmap, Mat& feat, const Mat& mask = Mat());
		};
	}
}



