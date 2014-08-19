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

	bool SegmentProcessor::ExtractBasicSegmentFeatures(SuperPixel& sp, const Mat& cimg, const Mat& dmap)
	{
		if(countNonZero(sp.mask) < 200)
			return false;

		// edge detection
		Mat edgemap;
		cv::Canny(sp.mask*150, edgemap, 100, 200);
		//imshow("edge", edgemap);
		//waitKey(0);

		Contours curves;
		std::vector<cv::Vec4i> hierarchy;
		findContours( edgemap, curves, hierarchy, CV_RETR_LIST, CV_CHAIN_APPROX_NONE );

		sp.contour = curves[0];
		approxPolyDP(sp.contour, sp.approx_contour, cv::arcLength(cv::Mat(sp.contour), true)*0.02, true);
		sp.area = countNonZero(sp.mask);
		sp.box = boundingRect(sp.approx_contour);
		sp.perimeter = arcLength(curves[0], true);
		sp.isConvex = isContourConvex(sp.approx_contour);
		sp.centroid.x = 0;
		sp.centroid.y = 0;
		for (int r=sp.box.y; r<sp.box.br().y; r++)
		{
			for(int c=sp.box.x; c<sp.box.br().x; c++)
			{
				sp.centroid.x += c*sp.mask.at<uchar>(r,c);
				sp.centroid.y += r*sp.mask.at<uchar>(r,c);
			}
		}
		sp.centroid.x /= sp.area;
		sp.centroid.y /= sp.area;
		
		sp.meanDepth = mean(dmap, sp.mask).val[0];

		return true;
	}

	bool SegmentProcessor::ExtractSegmentFeatures(SuperPixel& sp, const Mat& cimg, const Mat& dmap, int feattype)
	{
		//ExtractBasicSegmentFeatures(sp, cimg, dmap);

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
			dist += norm(p1->second, p2->second, NORM_L1);
		}
		dist /= sp1.feats.size();

		return dist;
	}
}



