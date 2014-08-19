//////////////////////////////////////////////////////////////////////////
// a class for visualization functions
// jiefeng©2014-3-26
//////////////////////////////////////////////////////////////////////////


#pragma once

#include "Common/common_libs.h"


namespace visualsearch
{
	namespace visualization
	{
		class ImgVisualizer
		{
		private:


		public:
			ImgVisualizer(void);

			// draw image windows
			static bool DrawWinsOnImg(string winname, const Mat& img, const vector<ImgWin>& wins, Mat& oimg = Mat());

			// draw grid of cropped images
			static bool DrawCroppedWins(string winname, const Mat& cimg, const vector<ImgWin>& wins, int numperrow, Mat& oimg = Mat());

			// visualize float precision image
			static bool DrawFloatImg(string winname, const Mat& img, Mat& oimg, bool toDraw = true);

			// draw contour of a shape
			static bool DrawShapes(const Mat& img, const vector<BasicShape>& shapes);

			// draw a grid of images
			static bool DrawImgCollection(string winname, const vector<Mat>& imgs, int numperrow, Mat& oimg);

		};
	}
	
}


