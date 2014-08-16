#include "ShapeAnalyzer.h"

namespace visualsearch
{
	namespace processors
	{
		ShapeAnalyzer::ShapeAnalyzer(void)
		{
		}

		//////////////////////////////////////////////////////////////////////////

		bool ShapeAnalyzer::FloodFillMask(const cv::Mat& grayimg, cv::Point& seed, float loDiff, float upDiff, cv::Mat& mask)
		{
			cv::Mat dst = grayimg.clone();
			mask.create(grayimg.rows+2, grayimg.cols+2, CV_8U);
			mask.setTo(0);

			// set params
			int ffillMode = 1;
			int connectivity = 4;
			int newMaskVal = 255;
			cv::Rect ccomp;

			int flags = connectivity + (newMaskVal << 8) +
				(ffillMode == 1 ? CV_FLOODFILL_FIXED_RANGE : 0);

			cv::Scalar newVal = cv::Scalar(255);
			int area = floodFill(dst, mask, seed, newVal, &ccomp, cv::Scalar(loDiff, loDiff, loDiff),
				cv::Scalar(upDiff, upDiff, upDiff), flags);
			
			Rect validroi(1, 1, mask.cols-2, mask.rows-2);
			mask(validroi).copyTo(mask);

			return true;
		}

		bool ShapeAnalyzer::ExtractConnectedComponents(const Mat& grayimg, vector<BasicShape>& shapes)
		{
			Mat grayimg_back = grayimg.clone();
			for (int r=0; r<grayimg_back.rows; r++)
			{
				for(int c=0; c<grayimg_back.cols; c++)
				{
					if(grayimg_back.at<uchar>(r,c) > 0)
					{
						// do flood fill to get first segment
						Mat mask;
						FloodFillMask(grayimg_back, Point(c, r), 0.5, 0.5, mask);
						BasicShape curshape;
						curshape.mask = mask;
						shapes.push_back(curshape);

						// set grayimg
						grayimg_back.setTo(0, mask);
					}
				}
			}

			return true;

			//////////////////////////////////////////////////////////////////////////
			// edge detection
			Mat edgemap;
			cv::Canny(grayimg*255, edgemap, 100, 200);

			Contours curves;
			std::vector<cv::Vec4i> hierarchy;
			findContours( edgemap, curves, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE );

			shapes.clear();
			shapes.reserve(2*curves.size());
			for(size_t i=0; i<curves.size(); i++)
			{
				BasicShape cur_shape;
				cur_shape.original_contour = curves[i];
				approxPolyDP(cur_shape.original_contour, cur_shape.approx_contour, cv::arcLength(cv::Mat(cur_shape.original_contour), true)*0.02, true);
				cur_shape.minRect = minAreaRect( cur_shape.approx_contour );
				cur_shape.bbox = boundingRect(cur_shape.approx_contour);
				// fill in mask
				cur_shape.mask.create(grayimg.rows, grayimg.cols, CV_8U);
				cur_shape.mask.setTo(0);
				Mat maskcontour = cur_shape.mask.clone();
				for(size_t j=0; j<cur_shape.original_contour.size(); j++)
					maskcontour.at<uchar>(cur_shape.original_contour[j]) = 1;
				for(int r=cur_shape.bbox.y; r<cur_shape.bbox.br().y; r++)
				{
					bool started = false;
					for(int c=cur_shape.bbox.x; c<cur_shape.bbox.br().x; c++)
					{
						if(maskcontour.at<uchar>(r,c) > 0)
						{
							started = !started;
							cur_shape.mask.at<uchar>(r,c) = 1;
						}
						if(started)
							cur_shape.mask.at<uchar>(r,c) = 1;
					}
				}

				cur_shape.area = contourArea(curves[i]);
				cur_shape.perimeter = arcLength(curves[i], true);
				cur_shape.isConvex = isContourConvex(cur_shape.approx_contour);
				shapes.push_back( cur_shape );
			}

			return true;
		}

	}
}


