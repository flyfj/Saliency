#include "SegmentProcessor.h"


namespace objectproposal
{
	SegmentProcessor::SegmentProcessor(void)
	{
		features::ColorFeatParams cparams;
		cparams.feat_type = features::COLOR_FEAT_HIST;
		cparams.histParams.color_space = features::COLOR_LAB;
		colorDesc.Init(cparams);

		features::EdgeFeatParams eparams;
		
		features::DepthFeatParams dparams;
		dparams.dtype = features::DEPTH_FEAT_MEAN;
		depthDesc.Init(dparams);
	}

	//////////////////////////////////////////////////////////////////////////

	bool SegmentProcessor::ExtractSegmentFeatures(SuperPixel& sp, const Mat& cimg, const Mat& dmap, int feattype)
	{
		sp.feats.clear();
		if( (feattype & SP_COLOR) != 0)
		{
			Mat curfeat;
			colorDesc.Compute(cimg, curfeat, sp.mask);
			sp.feats.push_back(curfeat);
		}
		if( (feattype & SP_TEXTURE) != 0)
		{
			Mat curfeat;
			Mat grayimg;
			cvtColor(cimg, grayimg, CV_BGR2GRAY);
			edgeDesc.Compute(grayimg, curfeat, sp.mask);
		}
		if( (feattype & SP_DEPTH) != 0 )
		{
			Mat curfeat;
			depthDesc.Compute(dmap, curfeat, sp.mask);
			sp.feats.push_back(curfeat);
		}

		return true;
	}

	float SegmentProcessor::ComputeSegmentDist(const SuperPixel& sp1, const SuperPixel& sp2)
	{
		float dist = 0;
		if(sp1.feats.size() != sp2.feats.size())
			return 1;
		for (size_t i=0; i<sp1.feats.size(); i++)
		{
			dist += norm(sp1.feats[i], sp2.feats[i], NORM_L2);
		}
		dist /= sp1.feats.size();

		return dist;
	}
}



