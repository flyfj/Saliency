//////////////////////////////////////////////////////////////////////////
// Class to generate different types of color based descriptors
// Jie Feng 2013-10-11
//////////////////////////////////////////////////////////////////////////


#pragma once

#include "common_libs.h"


namespace visualsearch
{
	namespace features
	{

		enum ColorFeatType
		{
			COLOR_FEAT_HIST,
			COLOR_FEAT_CLUSTER
		};

		enum FeatColorSpace
		{
			COLOR_RGB,
			COLOR_LAB,
			COLOR_HSV
		};

		struct ColorFeatParams
		{
			// params
			ColorFeatType feat_type;
			FeatColorSpace color_space;
			bool usePalette;

			// lab hist params
			int lab_bin_nums[4];	// l_bin_num; a_bin_num; b_bin_num; total_hist_bin_num

			// rgb hist params
			int rgb_bin_nums[4];

			ColorFeatParams()
			{
				InitParams(COLOR_RGB, COLOR_FEAT_HIST);
			}

			ColorFeatParams(FeatColorSpace cspace, ColorFeatType ctype, bool palette = false)
			{
				InitParams(cspace, ctype, palette);
			}

			void InitParams(FeatColorSpace cspace, ColorFeatType ctype, bool palette = false)
			{
				feat_type = ctype;
				color_space = cspace;
				usePalette = palette;

				if(cspace == COLOR_LAB && ctype == COLOR_FEAT_HIST)
				{
					lab_bin_nums[0] = lab_bin_nums[1] = lab_bin_nums[2] = 8;
					lab_bin_nums[3] = lab_bin_nums[0] * lab_bin_nums[1] * lab_bin_nums[2];
				}
				if(cspace == COLOR_RGB && ctype == COLOR_FEAT_HIST)
				{
					rgb_bin_nums[0] = rgb_bin_nums[1] = rgb_bin_nums[2] = 8;
					rgb_bin_nums[3] = rgb_bin_nums[0] * rgb_bin_nums[1] * rgb_bin_nums[2];
				}
			}
		};


		class ColorDescriptors
		{
		private:
			cv::Mat colorPalette;

		public:
			ColorDescriptors(void);

			// init
			bool Init();

			// processor
			// main entry
			bool Compute(const cv::Mat& color_img, cv::Mat& feat, 
				const ColorFeatParams& cparams, const cv::Mat& mask = cv::Mat());

			// generate a palette of color values in a specific color space
			// used as canonical color values to construct better(?) color histogram
			bool CreateColorPalette(FeatColorSpace colorspace, cv::Point3d nums);

			// generic interface for color histogram computation
			bool ComputeColorHistogram(const cv::Mat& color_img, cv::Mat& feat,
				const ColorFeatParams& cparams, const cv::Mat& mask = cv::Mat());
		};
	}
}


