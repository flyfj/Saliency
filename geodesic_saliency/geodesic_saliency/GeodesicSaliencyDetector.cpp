#include "GeodesicSaliencyDetector.h"


namespace Saliency
{

	GeodesicSaliencyDetector::GeodesicSaliencyDetector(void)
	{
	}

	bool GeodesicSaliencyDetector::Init(int imgWidth, int imgHeight, int imgStep)
	{
		int w = imgWidth;
		int h = imgHeight;
		int stride = imgStep;

		//initialize BGMap extractor
		//note that if we have multiple images to process, only need to initialize it once, 
		//but the iMaxWidth and iMaxHeight should be set to the maximum size of all images
		//for the grid size (iPatchWidth and iPatchHeight), usually if the image size is about 400*400, we can set the grid size to 10*10
		int iMaxWidth = w;
		int iMaxHeight = h;
		int iPatchWidth = 10;
		int iPatchHeight = 10;

		if ( !bgExtractor.Initialize(iMaxWidth, iMaxHeight, iPatchWidth, iPatchHeight))
		{
			printf("bgExtractor initialization error.\n");
			return false;
		}

		return true;
	}

	bool GeodesicSaliencyDetector::ComputeSaliencyMap(const Mat& img, Mat& smap)
	{
		Mat new_img;
		cv::cvtColor(img, new_img, CV_BGR2RGB);

		int w = img.cols;
		int h = img.rows;
		int stride = img.step;

		//allocate memory for saliency map
		float *pfBGMap = NULL;
		pfBGMap = new float[w * h];
		if (NULL == pfBGMap)
		{
			printf("memory error.\n");
			return false;
		}

		//generate saliency map (saliency values have been normalized to 0-1)
		if (!bgExtractor.BGMapExtract((const unsigned char *)img.data, w, h, stride, pfBGMap))
		{
			printf("BG_map extraction error.\n");
			delete[] pfBGMap;
			pfBGMap = NULL;
			return false;
		}

		//save saliency map
		//int sizes[] = {h,w};
		smap = Mat(h, w, CV_32F);
		memcpy(smap.data, pfBGMap, sizeof(float)*w*h);
		smap.convertTo(smap, CV_32F, 255);

		imwrite("saliencymap.jpg", smap);

		delete[] pfBGMap;
		pfBGMap = NULL;

		return true;
	}


}