//////////////////////////////////////////////////////////////////////////
// Class to generate different types of color based descriptors
// Jie Feng 2013-10-11
//////////////////////////////////////////////////////////////////////////


#pragma once

#include "../Common\common_libs.h"
#include "../Common\Tools.h"

namespace visualsearch
{
	namespace features
	{

		enum ColorFeatType
		{
			COLOR_FEAT_HIST,
			COLOR_FEAT_CLUSTER,
			COLOR_FEAT_MEAN
		};

		enum FeatColorSpace
		{
			COLOR_RGB,
			COLOR_LAB,
			COLOR_HSV
		};

		struct ColorCluster
		{
			Vec3f center_data;
			float center_weight;
		};

		struct ColorHistParams
		{
			FeatColorSpace color_space;
			bool usePalette;

			int hist_bin_nums[4];	// for 3 channel colors: 1_bin_num; 2_bin_num; 3_bin_num; total_hist_bin_num

			ColorHistParams()
			{ 
				color_space = COLOR_HSV;
				usePalette = false;
				hist_bin_nums[0] = 24; hist_bin_nums[1] = 8; hist_bin_nums[2] = 8;
				hist_bin_nums[3] = hist_bin_nums[0] + hist_bin_nums[1] + hist_bin_nums[2];	// TODO: sth wrong here
			}
		};

		struct ColorMeanParams
		{
			FeatColorSpace color_space;

			ColorMeanParams() { color_space = COLOR_LAB; }
		};

		struct ColorClusterParams
		{
			FeatColorSpace color_space;
			int cluster_num;
			double MAX_DIST;

			ColorClusterParams()
			{ color_space = COLOR_LAB; cluster_num = 8; MAX_DIST = 200; }
			ColorClusterParams(FeatColorSpace cspace, int cls_num)
			{ color_space = cspace; cluster_num = cls_num; MAX_DIST = 200; }
		};

		// generic color feature params
		struct ColorFeatParams
		{
			// params
			ColorFeatType feat_type;

			ColorHistParams histParams;
			ColorClusterParams clusterParams;
			ColorMeanParams meanparams;

			ColorFeatParams()
			{ feat_type = COLOR_FEAT_HIST; histParams = ColorHistParams(); }
			ColorFeatParams(ColorFeatType type)
			{ feat_type = type; }
		};


		class ColorDescriptors
		{
		private:
			
			// resize image
			int MAX_IMG_DIM;
			// params
			ColorFeatParams params;

		public:

			Mat colorPalette;	// 2d matrix, each component is a 3 channel color
			Mat grayPalette;	// 1d

			ColorDescriptors(void);

			// init
			bool Init(const ColorFeatParams& cparams);

			// processor
			// main entry
			bool Compute(const cv::Mat& color_img, cv::Mat& feat, const cv::Mat& mask = cv::Mat());

			// generate a palette of color values in a specific color space
			// used as canonical color values to construct better(?) color histogram
			bool CreateFixedColorPalette(FeatColorSpace colorspace, cv::Point3d nums);
			// map a pixel to palette color
			bool MapPixelToPalette(const Vec3f& hsv_val, const Vec3f& lab_val, Point& color_id, int& gray_id, bool& iscolor);

			// generic interface for color histogram computation
			bool ComputeColorHistogram(const cv::Mat& color_img, cv::Mat& feat, const cv::Mat& mask = cv::Mat());

			// compute mean color
			bool ComputeMeanColor(const cv::Mat& color_img, cv::Mat& feat, const cv::Mat& mask = Mat());

			// compute color cluster feature
			bool ComputeColorClusters(const cv::Mat& color_img, cv::Mat& feat, const cv::Mat& mask = cv::Mat());

			// use a approximate EMD distance
			float ColorClusterDist(const cv::Mat& feat1, const cv::Mat& feat2);

			//////////////////////////////////////////////////////////////////////////
			// tool: for processing, simple testing

			// learn color palette from input images (for single image, use computecluster function)
			bool LearnColorPalette(const vector<cv::Mat>& color_imgs, int maxNum, cv::Mat& palette);

			// visualize
			void DrawPalette(const cv::Mat& palette);
			// use compressed color to reconstruct image (used to check mapping quality)
			void MapImageToFixedPalette(const cv::Mat& color_img);
			void MapImageToLearnedPalette(const Mat& color_img);
			// compare histograms of different images
			void DrawColorHists(const vector<Mat>& cimgs);
		};
	}
}


