

#pragma once

#include "BGMapExtractor_Grid.h"
#include <opencv2\opencv.hpp>
using namespace cv;


namespace Saliency
{
	class GeodesicSaliencyDetector
	{
	private:
		BGMapExtractor_Grid bgExtractor;

	public:
		GeodesicSaliencyDetector(void);

		bool Init(int imgWidth, int imgHeight, int imgStep);

		bool ComputeSaliencyMap(const Mat& img, Mat& smap);
	};

}
