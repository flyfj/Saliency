//////////////////////////////////////////////////////////////////////////
//	collection of saliency computation methods for a given segment
//	jie feng @ 2012-12-14
//////////////////////////////////////////////////////////////////////////


#pragma once

#include "common.h"


namespace Saliency
{

	enum SaliencyType
	{
		Composition, CenterSurroundHistogramContrast
	};


	class SaliencyComputer
	{
	private:

		Mat lab_img;	// used for feature computation

	public:
		SaliencyComputer(void);
		~SaliencyComputer(void);

		// measure saliency of arbitrary segment within an image
		float ComputeSegmentSaliency(const Mat& img, const SegSuperPixelFeature& sp_feat, SaliencyType type);

	};

}


