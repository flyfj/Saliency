#include "DepthDescriptors.h"


namespace visualsearch
{
	namespace features
	{
		DepthDescriptors::DepthDescriptors(void)
		{
			params.dtype = DEPTH_FEAT_MEAN;
		}

		//////////////////////////////////////////////////////////////////////////

		bool DepthDescriptors::Init(const DepthFeatParams& dparams)
		{
			params = dparams;
			return true;
		}

		bool DepthDescriptors::ComputeMeanDepth(const Mat& dmap, Mat& feat, const Mat& mask)
		{
			Scalar meand = mean(dmap);
			feat.create(1, 1, CV_32F);
			feat.at<float>(0) = meand.val[0];

			return true;
		}

		bool DepthDescriptors::Compute(const Mat& dmap, Mat& feat, const Mat& mask)
		{
			if(params.dtype == DEPTH_FEAT_MEAN)
				return ComputeMeanDepth(dmap, feat, mask);

			return true;
		}
	}
}



