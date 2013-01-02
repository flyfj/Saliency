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

		bool InitCompositionFeature(vector<SegSuperPixelFeature>& prim_sp_feats);

		float Compose(const SegSuperPixelFeature& sp_feat, vector<SegSuperPixelFeature>& prim_sp_feats);

		static float Point2RectDistance(int x, int y, const Rect& box);
		static float HausdorffDist(Rect a, Rect b);

		// measure saliency of arbitrary segment within an image
		float ComputeSegmentSaliency(
			const Mat& img, 
			const SegSuperPixelFeature& sp_feat, 
			vector<SegSuperPixelFeature>& prim_sp_feats, 
			SaliencyType type);


	};

}


