#include "SegmentProcessor.h"


namespace objectproposal
{
	SegmentProcessor::SegmentProcessor(void)
	{
		features::ColorFeatParams cparams;
		cparams.feat_type = features::COLOR_FEAT_HIST;
		cparams.histParams.color_space = features::COLOR_RGB;
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
			sp.feats["color"] = curfeat;
		}
		if( (feattype & SP_TEXTURE) != 0)
		{
			Mat curfeat;
			Mat grayimg;
			cvtColor(cimg, grayimg, CV_BGR2GRAY);
			edgeDesc.Compute(grayimg, curfeat, sp.mask);
			sp.feats["texture"] = curfeat;
		}
		if( (feattype & SP_DEPTH) != 0 )
		{
			Mat curfeat;
			depthDesc.Compute(dmap, curfeat, sp.mask);
			sp.feats["depth"] = curfeat;
		}

		return true;
	}

	float SegmentProcessor::ComputeSegmentDist(const SuperPixel& sp1, const SuperPixel& sp2)
	{
		float dist = 0;
		if(sp1.feats.size() != sp2.feats.size())
			return 1;
		for (MatFeatureSet::const_iterator p1=sp1.feats.begin(), p2=sp2.feats.begin();
			p1!=sp1.feats.end(); p1++, p2++)
		{
			dist += norm(p1->second, p2->second, NORM_L2);
		}
		dist /= sp1.feats.size();

		return dist;
	}
}



